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
	AnimationEndDelegate.BindUFunction(this, "ReloadAnimationEndCallback");
}

// Called when the game starts or when spawned
void ABaseWeapon::BeginPlay() {
	Super::BeginPlay();
	CurrentAmmoInClip = CurrentAmmo < 0 ? ClipSize : FMath::Min(CurrentAmmo, ClipSize);

	if(!HasAuthority())
		return; // On Client the Instigator is not set yet

	OwningCharacter = Cast<ABaseCharacter>(Instigator);
	if(!AssertNotNull(OwningCharacter, GetWorld(), __FILE__, __LINE__))
		return;
	Recorder = OwningCharacter->FindComponentByClass<UPlayerStateRecorder>();
	ReattachMuzzleLocation();
}

void ABaseWeapon::OnRep_Instigator() {
	OwningCharacter = Cast<ABaseCharacter>(Instigator);
	if(!AssertNotNull(OwningCharacter, GetWorld(), __FILE__, __LINE__))
		return;
	Recorder = OwningCharacter->FindComponentByClass<UPlayerStateRecorder>();
	ReattachMuzzleLocation();
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
}

void ABaseWeapon::AttachToOwner() {
	DetachFromOwner();

	Mesh1P->AttachToComponent(OwningCharacter->Mesh1P, FAttachmentTransformRules(EAttachmentRule::SnapToTarget, true), TEXT("GripPoint"));
	Mesh3P->AttachToComponent(OwningCharacter->Mesh3P, FAttachmentTransformRules(EAttachmentRule::SnapToTarget, true), TEXT("GripPoint"));
}

// Called every frame
void ABaseWeapon::Tick(float DeltaTime) {
	Super::Tick(DeltaTime);
	if(!HasAuthority()) {
		return;
	}

	passedTime += DeltaTime;

	if(CurrentAmmoInClip == 0 && CurrentAmmo != 0 && CurrentState != EWeaponState::RELOADING) {
		StartTimer(this, GetWorld(), "Reload", 0.1f + ReloadDelay, false); // use timer to avoid reload animation loops
	} else if(CurrentState == EWeaponState::FIRING && passedTime - LastShotTimeStamp >= FireInterval) {
		passedTime = 0;
		Fire();
	} else if(Recorder && IsReplaySimulatedFirePressed) {
		IsReplaySimulatedFirePressed = false;
		Recorder->ServerRecordKeyReleased("WeaponFired");
	}
}

void ABaseWeapon::StartFire() {
	if(CurrentState == EWeaponState::IDLE) {
		CurrentState = EWeaponState::FIRING;
	}
}

void ABaseWeapon::Fire() {
	if(CurrentAmmo == 0 && CurrentAmmoInClip == 0) {
		SpawnNoAmmoSound();
	} else {
		if(CurrentAmmo == 0)
			return;

		if(Recorder) {
			IsReplaySimulatedFirePressed = true;
			Recorder->ServerRecordKeyPressed("WeaponFired");
		}
		PreExecuteFire();
		for(int i = 0; i < FireShotNum; i++)
			ExecuteFire();
		PostExecuteFire();
		CurrentAmmoInClip--;
		CurrentAmmo--;
		ForceNetUpdate();
		MulticastFireExecuted();
	}
	if(!AutoFire)
		CurrentState = EWeaponState::IDLE;
}

void ABaseWeapon::OnEquip() {
	if(!AssertNotNull(OwningCharacter, GetWorld(), __FILE__, __LINE__))
		return;
	equipped = true;
	passedTime = 0.f;
	SetActorTickEnabled(true);
	OwningCharacter->Mesh1P->GetAnimInstance()->Montage_Play(EquipAnimation1P);
	OwningCharacter->Mesh3P->GetAnimInstance()->Montage_Play(EquipAnimation3P);

	AttachToOwner();

	Mesh1P->SetHiddenInGame(false);
	Mesh3P->SetHiddenInGame(false);
	Mesh3P->bCastHiddenShadow = true;

	CurrentState = EWeaponState::IDLE;
}

void ABaseWeapon::OnUnequip() {
	equipped = false;
	SetActorTickEnabled(false);

	DetachFromOwner();

	Mesh1P->SetHiddenInGame(true);
	Mesh3P->SetHiddenInGame(true);
	Mesh3P->bCastHiddenShadow = false;

	OwningCharacter->Mesh1P->GetAnimInstance()->OnPlayMontageNotifyBegin.Remove(AnimationNotifyDelegate);
	OwningCharacter->Mesh1P->GetAnimInstance()->OnMontageEnded.Remove(AnimationEndDelegate);

	OwningCharacter->Mesh1P->GetAnimInstance()->Montage_Stop(0, ReloadAnimation1P);
}

void ABaseWeapon::MulticastFireExecuted_Implementation() {
	SpawnMuzzleFX();
	SpawnFireSound();
	PlayFireAnimation();
}

void ABaseWeapon::SpawnFireSound() {
	// try and play the sound if specified
	if(FireSound != NULL) {
		UGameplayStatics::SpawnSoundAttached(FireSound, OwningCharacter->Mesh1P);
	}
}

void ABaseWeapon::SpawnNoAmmoSound() {
	// try and play the sound if specified
	if(NoAmmoSound != NULL) {
		UGameplayStatics::SpawnSoundAttached(NoAmmoSound, OwningCharacter->Mesh1P);
	}
}

void ABaseWeapon::PlayFireAnimation() {
	// try and play a firing animation if specified
	if(FireAnimation != NULL) {
		// Get the animation object for the arms mesh
		UAnimInstance* AnimInstance = OwningCharacter->Mesh1P->GetAnimInstance();
		if(AnimInstance != NULL) {
			AnimInstance->Montage_Play(FireAnimation, 1.f);
		}
	}
	Mesh1P->PlayAnimation(FireAnimationWeapon1P, false);
}

void ABaseWeapon::StopFire() {
	if(CurrentState == EWeaponState::FIRING) {
		CurrentState = EWeaponState::IDLE;
	}
}

void ABaseWeapon::Reload() {
	if(CurrentState != EWeaponState::RELOADING && CurrentAmmoInClip != ClipSize && CurrentAmmoInClip != CurrentAmmo) {
		OwningCharacter->Mesh1P->GetAnimInstance()->OnPlayMontageNotifyBegin.AddUnique(AnimationNotifyDelegate);
		OwningCharacter->Mesh1P->GetAnimInstance()->OnMontageEnded.AddUnique(AnimationEndDelegate);

		CurrentState = EWeaponState::RELOADING;
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
		CurrentState = EWeaponState::IDLE;
	}
}

void ABaseWeapon::ReloadAnimationEndCallback(UAnimMontage* Montage, bool bInterrupted) {
	if(Montage == ReloadAnimation1P && CurrentState == EWeaponState::RELOADING) {
		CurrentState = EWeaponState::IDLE;
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

void ABaseWeapon::PreExecuteFire() {}
void ABaseWeapon::PostExecuteFire() {}
