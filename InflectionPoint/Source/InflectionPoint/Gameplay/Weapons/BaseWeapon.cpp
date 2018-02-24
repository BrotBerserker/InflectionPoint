// Fill out your copyright notice in the Description page of Project Settings.

#include "InflectionPoint.h"
#include "Gameplay/Characters/BaseCharacter.h"
#include "Gameplay/Characters/ReplayCharacterBase.h"
#include "BaseWeapon.h"


void ABaseWeapon::GetLifetimeReplicatedProps(TArray< FLifetimeProperty > & OutLifetimeProps) const {
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
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

	AnimationNotifyDelegate.BindUFunction(this, "ReloadAnimationNotifyCallback");
	AnimationEndDelegate.BindUFunction(this, "ReloadAnimationEndCallback");
}

// Called when the game starts or when spawned
void ABaseWeapon::BeginPlay() {
	Super::BeginPlay();
	OwningCharacter = Cast<ABaseCharacter>(Instigator);

	Recorder = OwningCharacter->FindComponentByClass<UPlayerStateRecorder>();
	CurrentAmmo = MaxAmmo;

	// Reattach MuzzleLocation from weapon to camera to prevent the weapon animation from moving the MuzzleLocation
	AttachToOwner();
	FP_MuzzleLocation->AttachToComponent(OwningCharacter->FirstPersonCameraComponent, FAttachmentTransformRules(EAttachmentRule::KeepWorld, true));
	if(!equipped) {
		DetachFromOwner();
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

	if(CurrentAmmo == 0) {
		StartTimer(this, GetWorld(), "Reload", 0.1f, false); // use timer to avoid reload animation loops
	} else if(CurrentState == EWeaponState::FIRING && passedTime - LastShotTimeStamp >= FireInterval) {
		Fire();
		passedTime = 0;
	} else if(Recorder) {
		Recorder->RecordKeyReleased("WeaponFired");
	}
}

void ABaseWeapon::StartFire() {
	if(CurrentState == EWeaponState::IDLE) {
		CurrentState = EWeaponState::FIRING;
	}
}

void ABaseWeapon::Fire() {
	if(CurrentAmmo == 0) {
		return;
	}

	if(Recorder) {
		Recorder->RecordKeyPressed("WeaponFired");
	}

	ExecuteFire();
	CurrentAmmo--;
	ForceNetUpdate();
	MulticastProjectileFired();
	if(!AutoFire)
		CurrentState = EWeaponState::IDLE;
}

void ABaseWeapon::OnEquip() {
	equipped = true;
	SetActorTickEnabled(true);
	OwningCharacter->Mesh1P->GetAnimInstance()->Montage_Play(EquipAnimation);

	AttachToOwner();

	Mesh1P->SetHiddenInGame(false);
	Mesh3P->SetHiddenInGame(false);

	CurrentState = EWeaponState::IDLE;
}

void ABaseWeapon::OnUnequip() {
	equipped = false;
	SetActorTickEnabled(false);

	DetachFromOwner();

	Mesh1P->SetHiddenInGame(true);
	Mesh3P->SetHiddenInGame(true);

	OwningCharacter->Mesh1P->GetAnimInstance()->OnPlayMontageNotifyBegin.Remove(AnimationNotifyDelegate);
	OwningCharacter->Mesh1P->GetAnimInstance()->OnMontageEnded.Remove(AnimationEndDelegate);

	OwningCharacter->Mesh1P->GetAnimInstance()->Montage_Stop(0, ReloadAnimation); 
}

void ABaseWeapon::MulticastProjectileFired_Implementation() {
	// try and play the sound if specified
	if(FireSound != NULL) {
		UGameplayStatics::SpawnSoundAttached(FireSound, OwningCharacter->Mesh1P);
	}

	// try and play a firing animation if specified
	if(FireAnimation != NULL) {
		// Get the animation object for the arms mesh
		UAnimInstance* AnimInstance = OwningCharacter->Mesh1P->GetAnimInstance();
		if(AnimInstance != NULL) {
			AnimInstance->Montage_Play(FireAnimation, 1.f);
		}
	}
}

void ABaseWeapon::StopFire() {
	if(CurrentState == EWeaponState::FIRING) {
		CurrentState = EWeaponState::IDLE;
	}
}

void ABaseWeapon::Reload() {
	if(CurrentState != EWeaponState::RELOADING && CurrentAmmo != MaxAmmo) {
		OwningCharacter->Mesh1P->GetAnimInstance()->OnPlayMontageNotifyBegin.AddUnique(AnimationNotifyDelegate);
		OwningCharacter->Mesh1P->GetAnimInstance()->OnMontageEnded.AddUnique(AnimationEndDelegate);

		CurrentState = EWeaponState::RELOADING;
		MulticastPlayReloadAnimation();
		OwningCharacter->Mesh1P->GetAnimInstance()->Montage_Play(ReloadAnimation);
	}
}

void ABaseWeapon::MulticastPlayReloadAnimation_Implementation() {
	if(!HasAuthority()) {
		OwningCharacter->Mesh1P->GetAnimInstance()->Montage_Play(ReloadAnimation);
	}
}

void ABaseWeapon::ReloadAnimationNotifyCallback(FName NotifyName, const FBranchingPointNotifyPayload& BranchingPointPayload) {
	if(NotifyName.ToString() == "RefillAmmo") {
		CurrentAmmo = MaxAmmo;
		ForceNetUpdate();
	}
}

void ABaseWeapon::ReloadAnimationEndCallback(UAnimMontage* Montage, bool bInterrupted) {
	if(Montage == ReloadAnimation) {
		CurrentState = EWeaponState::IDLE;
	}
}

FRotator ABaseWeapon::GetProjectileSpawnRotation() {
	return OwningCharacter->FirstPersonCameraComponent->GetComponentRotation();
}

FVector ABaseWeapon::GetProjectileSpawnLocation() {
	return ((FP_MuzzleLocation != nullptr) ? FP_MuzzleLocation->GetComponentLocation() : GetActorLocation());
}

