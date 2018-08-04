// Fill out your copyright notice in the Description page of Project Settings.

#include "InflectionPoint.h"
#include "Gameplay/Characters/BaseCharacter.h"
#include "Gameplay/Characters/ReplayCharacterBase.h"
#include "BaseWeapon.h"


void ABaseWeapon::GetLifetimeReplicatedProps(TArray< FLifetimeProperty > & OutLifetimeProps) const {DebugPrint(__FILE__, __LINE__);
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(ABaseWeapon, CurrentAmmoInClip);
	DOREPLIFETIME(ABaseWeapon, CurrentAmmo);
	DOREPLIFETIME(ABaseWeapon, OwningCharacter);
	DOREPLIFETIME(ABaseWeapon, CurrentState);
}

// Sets default values
ABaseWeapon::ABaseWeapon() {DebugPrint(__FILE__, __LINE__);
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;

	// Create a gun mesh component
	Mesh1P = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Mesh1P"));
	Mesh1P->SetOnlyOwnerSee(true);			// only the owning player will see this mesh
	Mesh1P->bCastDynamicShadow = false;
	Mesh1P->CastShadow = false;

	RootComponent = Mesh1P;

	Mesh1P->RelativeScale3D = FVector(.4, .4, .4);

	// Create the '3rd person' gun mesh
	Mesh3P = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Mesh3P"));
	Mesh3P->SetOwnerNoSee(true);
	Mesh3P->bCastHiddenShadow = true;

	// MuzzleLocation, where shots will be spawned
	FP_MuzzleLocation = CreateDefaultSubobject<USceneComponent>(TEXT("MuzzleLocation"));
	FP_MuzzleLocation->SetupAttachment(Mesh1P);
	FP_MuzzleLocation->SetRelativeLocation(FVector(0.2f, 60.f, 11.f));

	// MuzzleLocation that will be used instead when the player is aiming
	FP_Aim_MuzzleLocation = CreateDefaultSubobject<USceneComponent>(TEXT("FPAimMuzzleLocation"));
	FP_Aim_MuzzleLocation->SetupAttachment(Mesh1P);
	FP_Aim_MuzzleLocation->SetRelativeLocation(FVector(9.f, 81.f, 11.f));

	// MuzzleLocation for playing visual fx only
	TP_MuzzleLocation = CreateDefaultSubobject<USceneComponent>(TEXT("TPMuzzleLocation"));
	TP_MuzzleLocation->SetupAttachment(Mesh3P);
	TP_MuzzleLocation->SetRelativeLocation(FVector(0.7f, 56.f, 10.8f));

	AnimationNotifyDelegate.BindUFunction(this, "ReloadAnimationNotifyCallback");
	AnimationEndDelegate.BindUFunction(this, "ReloadAnimationEndCallback");

	AISuitabilityWeaponRangeCurve.GetRichCurve()->DefaultValue = 1.0;
	AISuitabilityWeaponRangeCurve.GetRichCurve()->AddKey(0, 1.0);
}

void ABaseWeapon::BeginPlay() {DebugPrint(__FILE__, __LINE__);
	Super::BeginPlay();
	CurrentAmmoInClip = CurrentAmmo < 0 ? ClipSize : FMath::Min(CurrentAmmo, ClipSize);

	if(!HasAuthority())
		return; // On Client the Instigator is not set yet

	Initialize();
}

void ABaseWeapon::OnRep_Instigator() {DebugPrint(__FILE__, __LINE__);
	Initialize();
}

void ABaseWeapon::Initialize() {DebugPrint(__FILE__, __LINE__);
	OwningCharacter = Cast<ABaseCharacter>(Instigator);
	AssertNotNull(OwningCharacter, GetWorld(), __FILE__, __LINE__);
	Recorder = OwningCharacter->FindComponentByClass<UPlayerStateRecorder>();
	//ReattachMuzzleLocation(); // doesnt work because the muzzle location would end up at the wrong location
	StartTimer(this, GetWorld(), "ReattachMuzzleLocation", 0.7f, false);
}

void ABaseWeapon::ReattachMuzzleLocation() {DebugPrint(__FILE__, __LINE__);
	// Reattach MuzzleLocation from weapon to camera
	AttachToOwner();
	FP_MuzzleLocation->AttachToComponent(OwningCharacter->FirstPersonCameraComponent, FAttachmentTransformRules(EAttachmentRule::KeepWorld, true));
	FP_Aim_MuzzleLocation->AttachToComponent(OwningCharacter->FirstPersonCameraComponent, FAttachmentTransformRules(EAttachmentRule::KeepWorld, true));
	if(!equipped) {DebugPrint(__FILE__, __LINE__);
		OnUnequip();
	}
}

void ABaseWeapon::DetachFromOwner() {DebugPrint(__FILE__, __LINE__);
	Mesh1P->DetachFromComponent(FDetachmentTransformRules::KeepRelativeTransform);
	Mesh3P->DetachFromComponent(FDetachmentTransformRules::KeepRelativeTransform);
}

void ABaseWeapon::AttachToOwner() {DebugPrint(__FILE__, __LINE__);
	DetachFromOwner();

	Mesh1P->AttachToComponent(OwningCharacter->Mesh1P, FAttachmentTransformRules(EAttachmentRule::SnapToTarget, true), TEXT("GripPoint"));
	Mesh3P->AttachToComponent(OwningCharacter->Mesh3P, FAttachmentTransformRules(EAttachmentRule::SnapToTarget, true), TEXT("GripPoint"));
}

void ABaseWeapon::Tick(float DeltaTime) {DebugPrint(__FILE__, __LINE__);
	Super::Tick(DeltaTime);
	if(!HasAuthority()) {DebugPrint(__FILE__, __LINE__);
		return;
	}

	timeSinceLastShot += DeltaTime;

	if(CurrentAmmoInClip == 0 && CurrentAmmo != 0 && CurrentState != EWeaponState::RELOADING 
		&& CurrentState != EWeaponState::EQUIPPING && timeSinceLastShot >= ReloadDelay) {DebugPrint(__FILE__, __LINE__);
		StartTimer(this, GetWorld(), "Reload", 0.1f, false); // use timer to avoid reload animation loops
	} else if(CurrentState == EWeaponState::FIRING && timeSinceLastShot >= FireInterval) {DebugPrint(__FILE__, __LINE__);
		Fire();
	} else if(Recorder && RecordKeyReleaseNextTick) {DebugPrint(__FILE__, __LINE__);
		RecordKeyReleaseNextTick = false;
		Recorder->ServerRecordKeyReleased("WeaponFired");
	}
}

void ABaseWeapon::StartFire() {DebugPrint(__FILE__, __LINE__);
	if(CurrentAmmo == 0 && CurrentAmmoInClip == 0) {DebugPrint(__FILE__, __LINE__);
		MulticastSpawnNoAmmoSound();
	} else if(CurrentState == EWeaponState::IDLE && CurrentAmmoInClip > 0) {DebugPrint(__FILE__, __LINE__);
		ChangeWeaponState(EWeaponState::FIRING);
	}
}

void ABaseWeapon::FireOnce() {DebugPrint(__FILE__, __LINE__);
	if(CurrentAmmo == 0 && CurrentAmmoInClip == 0) {DebugPrint(__FILE__, __LINE__);
		MulticastSpawnNoAmmoSound();
	} else if(CurrentState == EWeaponState::IDLE && CurrentAmmoInClip > 0 && timeSinceLastShot >= FireInterval) {DebugPrint(__FILE__, __LINE__);
		ChangeWeaponState(EWeaponState::FIRING);
		Fire();
		ChangeWeaponState(EWeaponState::IDLE);
	}
}

void ABaseWeapon::Fire() {DebugPrint(__FILE__, __LINE__);
	if(Recorder) {DebugPrint(__FILE__, __LINE__);
		RecordKeyReleaseNextTick = true;
		Recorder->ServerRecordKeyPressed("WeaponFired");
	}
	if(CurrentAmmoInClip <= 0) 
		return;
	timeSinceLastShot = 0;
	PreExecuteFire();
	for(int i = 0; i < FireShotNum; i++)
		ExecuteFire();
	PostExecuteFire();
	CurrentAmmoInClip--;
	CurrentAmmo--;
	ForceNetUpdate();
	MulticastFireExecuted();
	if(!AutoFire)
		ChangeWeaponState(EWeaponState::IDLE);
}

void ABaseWeapon::OnEquip() {DebugPrint(__FILE__, __LINE__);
	timeSinceLastShot = FireInterval; // so you can fire after EquipDelay
	ChangeWeaponState(EWeaponState::EQUIPPING);

	OwningCharacter->Mesh1P->GetAnimInstance()->Montage_Play(EquipAnimation1P);
	OwningCharacter->Mesh3P->GetAnimInstance()->Montage_Play(EquipAnimation3P);

	UpdateEquippedState(true);

	StartTimer(this, GetWorld(), "ChangeWeaponState", EquipDelay+0.001f, false, EWeaponState::IDLE);

	if(OwningCharacter->IsAiming) {DebugPrint(__FILE__, __LINE__);
		StartAiming();
	}
}

void ABaseWeapon::OnUnequip() {DebugPrint(__FILE__, __LINE__);
	if(OwningCharacter->IsAiming) {DebugPrint(__FILE__, __LINE__);
		StopAiming();
	}

	UpdateEquippedState(false);

	OwningCharacter->Mesh1P->GetAnimInstance()->OnPlayMontageNotifyBegin.Remove(AnimationNotifyDelegate);
	OwningCharacter->Mesh1P->GetAnimInstance()->OnMontageEnded.Remove(AnimationEndDelegate);

	OwningCharacter->Mesh1P->GetAnimInstance()->Montage_Stop(0, ReloadAnimation1P);
}

void ABaseWeapon::UpdateEquippedState(bool newEquipped) {DebugPrint(__FILE__, __LINE__);
	this->equipped = newEquipped;
	SetActorTickEnabled(newEquipped);
	Mesh1P->SetHiddenInGame(!newEquipped, true);
	Mesh3P->SetHiddenInGame(!newEquipped, true);
	Mesh3P->bCastHiddenShadow = newEquipped;
	newEquipped ? AttachToOwner() : DetachFromOwner();
}

void ABaseWeapon::MulticastFireExecuted_Implementation() {DebugPrint(__FILE__, __LINE__);
	SpawnMuzzleFX();
	SpawnFireSound();
	PlayFireAnimation();
	OnFireExecuted.Broadcast();
}

void ABaseWeapon::SpawnFireSound() {DebugPrint(__FILE__, __LINE__);
	UGameplayStatics::SpawnSoundAttached(FireSound, OwningCharacter->Mesh1P);
}

void ABaseWeapon::MulticastSpawnNoAmmoSound_Implementation() {DebugPrint(__FILE__, __LINE__);
	UGameplayStatics::SpawnSoundAttached(NoAmmoSound, OwningCharacter->Mesh1P);
}

void ABaseWeapon::PlayFireAnimation() {DebugPrint(__FILE__, __LINE__);
	UAnimInstance* AnimInstance = OwningCharacter->Mesh1P->GetAnimInstance();
	if(AnimInstance != NULL) {DebugPrint(__FILE__, __LINE__);
		AnimInstance->Montage_Play(FireAnimation, 1.f);
	}
	Mesh1P->PlayAnimation(FireAnimationWeapon1P, false);
}

void ABaseWeapon::StopFire() {DebugPrint(__FILE__, __LINE__);
	if(CurrentState == EWeaponState::FIRING) {DebugPrint(__FILE__, __LINE__);
		ChangeWeaponState(EWeaponState::IDLE);
	}
}

void ABaseWeapon::Reload() {DebugPrint(__FILE__, __LINE__);
	if(CurrentState != EWeaponState::RELOADING && CurrentState != EWeaponState::EQUIPPING && CurrentAmmoInClip != ClipSize && CurrentAmmoInClip != CurrentAmmo) {DebugPrint(__FILE__, __LINE__);
		OwningCharacter->Mesh1P->GetAnimInstance()->OnPlayMontageNotifyBegin.AddUnique(AnimationNotifyDelegate);
		OwningCharacter->Mesh1P->GetAnimInstance()->OnMontageEnded.AddUnique(AnimationEndDelegate);

		ChangeWeaponState(EWeaponState::RELOADING);
		MulticastPlayReloadAnimation();
		OwningCharacter->Mesh1P->GetAnimInstance()->Montage_Play(ReloadAnimation1P);
		OwningCharacter->Mesh3P->GetAnimInstance()->Montage_Play(ReloadAnimation3P);
		Mesh1P->PlayAnimation(ReloadAnimationWeapon1P, false);
	}
}

void ABaseWeapon::MulticastPlayReloadAnimation_Implementation() {DebugPrint(__FILE__, __LINE__);
	if(!HasAuthority()) {DebugPrint(__FILE__, __LINE__);
		OwningCharacter->Mesh1P->GetAnimInstance()->Montage_Play(ReloadAnimation1P);
		OwningCharacter->Mesh3P->GetAnimInstance()->Montage_Play(ReloadAnimation3P);
		Mesh1P->PlayAnimation(ReloadAnimationWeapon1P, false);
	}
}

void ABaseWeapon::ReloadAnimationNotifyCallback(FName NotifyName, const FBranchingPointNotifyPayload& BranchingPointPayload) {DebugPrint(__FILE__, __LINE__);
	if(NotifyName.ToString() == "RefillAmmo") {DebugPrint(__FILE__, __LINE__);
		CurrentAmmoInClip = CurrentAmmo < 0 ? ClipSize : FMath::Min(CurrentAmmo, ClipSize);
		ForceNetUpdate();
	} else if(NotifyName.ToString() == "EnableFiring") {DebugPrint(__FILE__, __LINE__);
		ChangeWeaponState(EWeaponState::IDLE);
	}
}

void ABaseWeapon::ReloadAnimationEndCallback(UAnimMontage* Montage, bool bInterrupted) {DebugPrint(__FILE__, __LINE__);
	if(Montage == ReloadAnimation1P && CurrentState == EWeaponState::RELOADING) {DebugPrint(__FILE__, __LINE__);
		ChangeWeaponState(EWeaponState::IDLE);
	}
}

void ABaseWeapon::SpawnMuzzleFX() {DebugPrint(__FILE__, __LINE__);
	if(!MuzzleFX)
		return;

	UParticleSystemComponent* mesh1pFX = UGameplayStatics::SpawnEmitterAttached(MuzzleFX, Mesh1P, NAME_None);
	if(mesh1pFX) {DebugPrint(__FILE__, __LINE__);
		mesh1pFX->SetRelativeScale3D(MuzzleFXScale);
		mesh1pFX->SetWorldLocation(GetFPMuzzleLocation());
		mesh1pFX->SetWorldRotation(GetAimDirection());
		mesh1pFX->bOwnerNoSee = false;
		mesh1pFX->bOnlyOwnerSee = true;
		if(MuzzleFXDuration > 0)
			StartTimer(this, GetWorld(), "DecativateParticleSystem", MuzzleFXDuration, false, mesh1pFX);
	}
	UParticleSystemComponent* mesh3pFX = UGameplayStatics::SpawnEmitterAttached(MuzzleFX, Mesh3P, NAME_None);
	if(mesh3pFX) {DebugPrint(__FILE__, __LINE__);
		mesh3pFX->SetWorldLocation(GetTPMuzzleLocation());
		mesh3pFX->SetWorldRotation(GetAimDirection());
		mesh3pFX->bOwnerNoSee = true;
		mesh3pFX->bOnlyOwnerSee = false;
		if(MuzzleFXDuration > 0)
			StartTimer(this, GetWorld(), "DecativateParticleSystem", MuzzleFXDuration, false, mesh3pFX);
	}
}

void ABaseWeapon::DecativateParticleSystem(UParticleSystemComponent* effect) {DebugPrint(__FILE__, __LINE__);
	effect->DeactivateSystem();
}

FRotator ABaseWeapon::GetAimDirection() {DebugPrint(__FILE__, __LINE__);
	return OwningCharacter->FirstPersonCameraComponent->GetComponentRotation();
}

FVector ABaseWeapon::GetFPMuzzleLocation() {DebugPrint(__FILE__, __LINE__);
	if(OwningCharacter && OwningCharacter->IsAiming) {DebugPrint(__FILE__, __LINE__);
		return FP_Aim_MuzzleLocation->GetComponentLocation();
	}
	return FP_MuzzleLocation->GetComponentLocation();
}

FVector ABaseWeapon::GetTPMuzzleLocation() {DebugPrint(__FILE__, __LINE__);
	return TP_MuzzleLocation->GetComponentLocation();
}

void ABaseWeapon::StartAiming() {DebugPrint(__FILE__, __LINE__);
	if(HideWeaponWhenAiming)
		Mesh1P->SetVisibility(false, true);
}

void ABaseWeapon::StopAiming() {DebugPrint(__FILE__, __LINE__);
	if(HideWeaponWhenAiming)
		Mesh1P->SetVisibility(true, true);
}

void ABaseWeapon::ChangeWeaponState(EWeaponState newState) {DebugPrint(__FILE__, __LINE__);
	CurrentState = newState;
}

EWeaponState ABaseWeapon::GetCurrentWeaponState() {DebugPrint(__FILE__, __LINE__);
	return CurrentState;
}

float ABaseWeapon::GetAIWeaponSuitability(ABaseCharacter* shooter, AActor* victim) {DebugPrint(__FILE__, __LINE__);
	if(CurrentAmmo == 0)
		return 0;
	float distance = 1000;
	if(shooter && victim)
		distance = (shooter->GetActorLocation() - victim->GetActorLocation()).Size();
	return AISuitabilityWeaponRangeCurve.GetRichCurveConst()->Eval(distance,0);
	//return 1.0;
}

void ABaseWeapon::PreExecuteFire() {DebugPrint(__FILE__, __LINE__);}
void ABaseWeapon::PostExecuteFire() {DebugPrint(__FILE__, __LINE__);}
