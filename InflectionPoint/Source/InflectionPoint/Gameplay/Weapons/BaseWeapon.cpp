// Fill out your copyright notice in the Description page of Project Settings.

#include "InflectionPoint.h"
#include "Gameplay/Characters/BaseCharacter.h"
#include "Gameplay/Characters/ReplayCharacterBase.h"
#include "BaseWeapon.h"


void ABaseWeapon::GetLifetimeReplicatedProps(TArray< FLifetimeProperty > & OutLifetimeProps) const {
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(ABaseWeapon, CurrentAmmoInClip);
	DOREPLIFETIME(ABaseWeapon, CurrentAmmo);
	DOREPLIFETIME(ABaseWeapon, OwningCharacter);
	DOREPLIFETIME(ABaseWeapon, CurrentState);
	DOREPLIFETIME(ABaseWeapon, SelectedTargetComponent);
	DOREPLIFETIME(ABaseWeapon, timeSinceLastShot);
}

// Sets default values
ABaseWeapon::ABaseWeapon() {
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

	AISuitabilityWeaponRangeCurve.GetRichCurve()->DefaultValue = 1.0;
	AISuitabilityWeaponRangeCurve.GetRichCurve()->AddKey(0, 1.0);
}

void ABaseWeapon::BeginPlay() {
	Super::BeginPlay();
	CurrentAmmoInClip = CurrentAmmo < 0 ? ClipSize : FMath::Min(CurrentAmmo, ClipSize);

	if(!HasAuthority())
		return; // On Client the Instigator is not set yet

	SetupReferences();
}

void ABaseWeapon::OnRep_Instigator() {
	SetupReferences();
}

void ABaseWeapon::SetupReferences() {
	OwningCharacter = Cast<ABaseCharacter>(Instigator);
	AssertNotNull(OwningCharacter, GetWorld(), __FILE__, __LINE__);
	Recorder = OwningCharacter->FindComponentByClass<UPlayerStateRecorder>();
	//ReattachMuzzleLocation(); // doesnt work because the muzzle location would end up at the wrong location
	StartTimer(this, GetWorld(), "ReattachMuzzleLocation", 0.7f, false);
}

bool ABaseWeapon::IsReadyForInitialization() {
	return OwningCharacter != nullptr;
}

void ABaseWeapon::ReattachMuzzleLocation() {
	// Reattach MuzzleLocation from weapon to camera
	AttachToOwner();
	FP_MuzzleLocation->AttachToComponent(OwningCharacter->FirstPersonCameraComponent, FAttachmentTransformRules(EAttachmentRule::KeepWorld, true));
	FP_Aim_MuzzleLocation->AttachToComponent(OwningCharacter->FirstPersonCameraComponent, FAttachmentTransformRules(EAttachmentRule::KeepWorld, true));
	if(!equipped) {
		OnUnequip();
	}
}

void ABaseWeapon::DetachFromOwner() {
	Mesh1P->DetachFromComponent(FDetachmentTransformRules::KeepRelativeTransform);
	Mesh3P->DetachFromComponent(FDetachmentTransformRules::KeepRelativeTransform);
	StopFire();
}

void ABaseWeapon::AttachToOwner() {
	DetachFromOwner();
	if(!OwningCharacter) {
		return;
	}
	Mesh1P->AttachToComponent(OwningCharacter->Mesh1P, FAttachmentTransformRules(EAttachmentRule::SnapToTarget, true), TEXT("GripPoint"));
	Mesh3P->AttachToComponent(OwningCharacter->Mesh3P, FAttachmentTransformRules(EAttachmentRule::SnapToTarget, true), TEXT("GripPoint"));
}

void ABaseWeapon::Tick(float DeltaTime) {
	Super::Tick(DeltaTime);
	if(HasAuthority()) {
		timeSinceLastShot += DeltaTime;
		timeSinceStartFire += DeltaTime;

		if(CurrentAmmoInClip == 0 && CurrentAmmo != 0 && CurrentState != EWeaponState::RELOADING
			&& CurrentState != EWeaponState::EQUIPPING && timeSinceLastShot >= ReloadDelay) {
			StartTimer(this, GetWorld(), "Reload", 0.1f, false); // use timer to avoid reload animation loops
		} else if(CurrentState == EWeaponState::CHARGING && timeSinceStartFire >= ChargeDuration) {
			ChangeWeaponState(EWeaponState::FIRING);
		} else if(CurrentState == EWeaponState::FIRING && timeSinceLastShot >= FireInterval) {
			Fire();
		} else if(Recorder && RecordKeyReleaseNextTick) {
			RecordKeyReleaseNextTick = false;
			Recorder->ServerRecordKeyReleased("WeaponFired");
		}
	}
}

void ABaseWeapon::StartFire() {
	wantsToFire = true;
	timeSinceStartFire = 0;
	if(CurrentAmmo == 0 && CurrentAmmoInClip == 0) {
		MulticastSpawnNoAmmoSound();
	} else if(CurrentState == EWeaponState::IDLE && CurrentAmmoInClip > 0) {
		MulticastStartStopChargeSound(true);
		ChangeWeaponState(EWeaponState::CHARGING);
	}
}

void ABaseWeapon::FireOnce() {
	if(CurrentAmmo == 0 && CurrentAmmoInClip == 0) {
		MulticastSpawnNoAmmoSound();
	} else if(CurrentState == EWeaponState::IDLE && CurrentAmmoInClip > 0 && timeSinceLastShot >= FireInterval) {
		ChangeWeaponState(EWeaponState::FIRING); // No charging for replays
		Fire();
		ChangeWeaponState(EWeaponState::IDLE);
	}
}

void ABaseWeapon::Fire() {
	if(Recorder) {
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

void ABaseWeapon::OnEquip() {
	timeSinceLastShot = FireInterval; // so you can fire after EquipDelay
	ChangeWeaponState(EWeaponState::EQUIPPING);

	UpdateEquippedState(true);

	StartTimer(this, GetWorld(), "ChangeWeaponState", EquipDelay + 0.001f, false, EWeaponState::IDLE);
}

void ABaseWeapon::OnUnequip() {
	wantsToFire = false;
	UpdateEquippedState(false);
}

void ABaseWeapon::UpdateEquippedState(bool newEquipped) {
	this->equipped = newEquipped;
	SetActorTickEnabled(newEquipped);
	Mesh1P->SetHiddenInGame(!newEquipped, true);
	Mesh3P->SetHiddenInGame(!newEquipped, true);
	Mesh3P->bCastHiddenShadow = newEquipped;
	newEquipped ? AttachToOwner() : DetachFromOwner();
}

void ABaseWeapon::MulticastFireExecuted_Implementation() {
	UGameplayStatics::PlayWorldCameraShake(GetWorld(), FireCameraShake, OwningCharacter->GetActorLocation(), 50, 60);
	SpawnMuzzleFX();
	SpawnFireSound();
	PlayFireAnimation();
	OnFireExecuted.Broadcast();
}

void ABaseWeapon::SpawnFireSound() {
	UGameplayStatics::SpawnSoundAttached(FireSound, OwningCharacter->Mesh1P);
	MulticastStartStopFireLoopSound(true);
}

void ABaseWeapon::MulticastSpawnNoAmmoSound_Implementation() {
	UGameplayStatics::SpawnSoundAttached(NoAmmoSound, OwningCharacter->Mesh1P);
}

void ABaseWeapon::MulticastStartStopChargeSound_Implementation(bool shouldPlay) {
	if(!ChargeSoundComponent)
		ChargeSoundComponent = UGameplayStatics::SpawnSoundAttached(ChargeSound, OwningCharacter->Mesh1P);
	if(!ChargeSoundComponent)
		return;
	if(shouldPlay) {
		ChargeSoundComponent->Play(0);
	} else{ 
		if(ChargeSoundComponent->IsPlaying()) 
			ChargeSoundComponent->FadeOut(0.2, 0);
		ChargeSoundComponent = nullptr;
	}
}

void ABaseWeapon::MulticastStartStopFireLoopSound_Implementation(bool shouldPlay) {
	if(!FireLoopSoundComponent) {
		FireLoopSoundComponent = UGameplayStatics::SpawnSoundAttached(FireLoopSound, OwningCharacter->Mesh1P);
		if(!shouldPlay && FireLoopSoundComponent)
			FireLoopSoundComponent->Stop(); // to prevent fadeout
	}
	if(!FireLoopSoundComponent)
		return;
	if(shouldPlay) {
		if(!FireLoopSoundComponent->IsPlaying())
			FireLoopSoundComponent->Play(0);
	} else {
		if(FireLoopSoundComponent->IsPlaying())
			FireLoopSoundComponent->FadeOut(0.2, 0);
		FireLoopSoundComponent = nullptr;
	}
}

void ABaseWeapon::PlayFireAnimation() {
	UAnimInstance* AnimInstance = OwningCharacter->Mesh1P->GetAnimInstance();
	if(AnimInstance != NULL) {
		AnimInstance->Montage_Play(FireAnimation, 1.f);
	}
	Mesh1P->PlayAnimation(FireAnimationWeapon1P, false);
}

void ABaseWeapon::StopFire() {
	wantsToFire = false;
	if(CurrentState == EWeaponState::FIRING || CurrentState == EWeaponState::CHARGING) {
		MulticastStartStopChargeSound(false);
		MulticastStartStopFireLoopSound(false);
		ChangeWeaponState(EWeaponState::IDLE);
	}
}

void ABaseWeapon::Reload() {
	MulticastStartStopFireLoopSound(false);
	if(CurrentState != EWeaponState::RELOADING && CurrentState != EWeaponState::EQUIPPING && CurrentAmmoInClip != ClipSize && CurrentAmmoInClip != CurrentAmmo) {
		OwningCharacter->Mesh1P->GetAnimInstance()->OnPlayMontageNotifyBegin.AddUnique(AnimationNotifyDelegate);

		ChangeWeaponState(EWeaponState::RELOADING);
		MulticastPlayReloadAnimation();
		OwningCharacter->Mesh1P->GetAnimInstance()->Montage_Play(ReloadAnimation1P);
		OwningCharacter->Mesh3P->GetAnimInstance()->Montage_Play(ReloadAnimation3P);
		Mesh1P->PlayAnimation(ReloadAnimationWeapon1P, false);
	}
}

void ABaseWeapon::MulticastPlayReloadAnimation_Implementation() {
	if(!HasAuthority()) {
		OwningCharacter->Mesh1P->GetAnimInstance()->Montage_Play(ReloadAnimation1P);
		OwningCharacter->Mesh3P->GetAnimInstance()->Montage_Play(ReloadAnimation3P);
		Mesh1P->PlayAnimation(ReloadAnimationWeapon1P, false);
	}
}

void ABaseWeapon::ReloadAnimationNotifyCallback(FName NotifyName, const FBranchingPointNotifyPayload& BranchingPointPayload) {
	if(NotifyName.ToString() == "RefillAmmo") {
		CurrentAmmoInClip = CurrentAmmo < 0 ? ClipSize : FMath::Min(CurrentAmmo, ClipSize);
		ForceNetUpdate();
	} else if(NotifyName.ToString() == "EnableFiring") {
		if(wantsToFire) {
			timeSinceStartFire = 0;
			MulticastStartStopChargeSound(true);
			ChangeWeaponState(EWeaponState::CHARGING);
		} else {
			ChangeWeaponState(EWeaponState::IDLE);
		}
	}
}

void ABaseWeapon::SpawnMuzzleFX() {
	if(!MuzzleFX)
		return;

	UParticleSystemComponent* mesh1pFX = UGameplayStatics::SpawnEmitterAttached(MuzzleFX, Mesh1P, NAME_None);
	if(mesh1pFX) {
		mesh1pFX->SetRelativeScale3D(MuzzleFXScale);
		mesh1pFX->SetWorldLocation(GetFPMuzzleLocation());
		mesh1pFX->SetWorldRotation(GetAimDirection());
		mesh1pFX->bOwnerNoSee = false;
		mesh1pFX->bOnlyOwnerSee = true;
		if(MuzzleFXDuration > 0)
			StartTimer(this, GetWorld(), "DecativateParticleSystem", MuzzleFXDuration, false, mesh1pFX);
	}
	UParticleSystemComponent* mesh3pFX = UGameplayStatics::SpawnEmitterAttached(MuzzleFX, Mesh3P, NAME_None);
	if(mesh3pFX) {
		mesh3pFX->SetWorldLocation(GetTPMuzzleLocation());
		mesh3pFX->SetWorldRotation(GetAimDirection());
		mesh3pFX->bOwnerNoSee = true;
		mesh3pFX->bOnlyOwnerSee = false;
		if(MuzzleFXDuration > 0)
			StartTimer(this, GetWorld(), "DecativateParticleSystem", MuzzleFXDuration, false, mesh3pFX);
	}
}

void ABaseWeapon::DecativateParticleSystem(UParticleSystemComponent* effect) {
	effect->DeactivateSystem();
}

FRotator ABaseWeapon::GetAimDirection() {
	return OwningCharacter->FirstPersonCameraComponent->GetComponentRotation();
}

FVector ABaseWeapon::GetFPMuzzleLocation() {
	if(OwningCharacter && OwningCharacter->IsAiming) {
		return FP_Aim_MuzzleLocation->GetComponentLocation();
	}
	return FP_MuzzleLocation->GetComponentLocation();
}

FVector ABaseWeapon::GetTPMuzzleLocation() {
	return TP_MuzzleLocation->GetComponentLocation();
}

void ABaseWeapon::StartAiming() {
	if(HideWeaponWhenAiming)
		Mesh1P->SetVisibility(false, true);
}

void ABaseWeapon::StopAiming() {
	if(HideWeaponWhenAiming)
		Mesh1P->SetVisibility(true, true);
}

void ABaseWeapon::ChangeWeaponState(EWeaponState newState) {
	CurrentState = newState;
}

EWeaponState ABaseWeapon::GetCurrentWeaponState() {
	return CurrentState;
}

float ABaseWeapon::GetAIWeaponSuitability(ABaseCharacter* shooter, AActor* victim) {
	if(CurrentAmmo == 0)
		return 0;
	float distance = 1000;
	if(shooter && victim)
		distance = (shooter->GetActorLocation() - victim->GetActorLocation()).Size();
	return AISuitabilityWeaponRangeCurve.GetRichCurveConst()->Eval(distance, 0);
	//return 1.0;
}

bool ABaseWeapon::ServerIncreaseCurrentAmmo_Validate(int amount) {
	return true;
}

void ABaseWeapon::ServerIncreaseCurrentAmmo_Implementation(int amount) {
	if(MaxAmmo >= 0 && CurrentAmmo + amount > MaxAmmo) {
		CurrentAmmo = MaxAmmo;
		return;
	}
	if(CurrentAmmo <= -1)
		return;
	CurrentAmmo += amount;
}

void ABaseWeapon::PreExecuteFire() {}
void ABaseWeapon::PostExecuteFire() {}
