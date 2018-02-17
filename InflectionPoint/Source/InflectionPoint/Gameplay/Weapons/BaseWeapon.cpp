// Fill out your copyright notice in the Description page of Project Settings.

#include "InflectionPoint.h"
#include "Gameplay/Characters/BaseCharacter.h"
#include "BaseWeapon.h"


void ABaseWeapon::GetLifetimeReplicatedProps(TArray< FLifetimeProperty > & OutLifetimeProps) const {
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ABaseWeapon, CurrentAmmo);
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

	//Mesh1P->SetupAttachment(GetCapsuleComponent());

	Mesh1P->RelativeScale3D = FVector(.4, .4, .4);

	// Create the '3rd person' gun mesh
	Mesh3P = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Mesh3P"));
	Mesh3P->SetOwnerNoSee(true);
	//Mesh3P->SetupAttachment(GetCapsuleComponent());

	// MuzzleLocation, where shots will be spawned
	FP_MuzzleLocation = CreateDefaultSubobject<USceneComponent>(TEXT("MuzzleLocation"));
	FP_MuzzleLocation->SetupAttachment(Mesh1P);
	/*FP_MuzzleLocation->SetRelativeLocation(FVector(0.2f, 48.4f, -10.6f));*/
	//FP_MuzzleLocation->SetRelativeLocation(FVector(0.2f, 172.f, 11.f));
	FP_MuzzleLocation->SetRelativeLocation(FVector(0.2f, 60.f, 11.f));
	
}

// Called when the game starts or when spawned
void ABaseWeapon::BeginPlay() {
	Super::BeginPlay();
	OwningCharacter = Cast<ABaseCharacter>(Instigator);
	CurrentAmmo = MaxAmmo;
}

// Called every frame
void ABaseWeapon::Tick(float DeltaTime) {
	Super::Tick(DeltaTime);
	if(!HasAuthority()) {
		return;
	}
	if(CurrentAmmo == 0) {
		Reload();
	} else if(CurrentState == EWeaponState::FIRING && UGameplayStatics::GetRealTimeSeconds(GetWorld()) - LastShotTimeStamp >= FireInterval) {
		Fire();
		LastShotTimeStamp = UGameplayStatics::GetRealTimeSeconds(GetWorld());
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
	ExecuteFire();
	CurrentAmmo--;
	MulticastProjectileFired();
	if(!AutoFire)
		CurrentState = EWeaponState::IDLE;
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
		CurrentState = EWeaponState::RELOADING;
		MulticastPlayReloadAnimation();
		OwningCharacter->Mesh1P->GetAnimInstance()->Montage_Play(ReloadAnimation);

		FScriptDelegate NotifyDelegate;
		NotifyDelegate.BindUFunction(this, "ReloadAnimationNotifyCallback");
		OwningCharacter->Mesh1P->GetAnimInstance()->OnPlayMontageNotifyBegin.AddUnique(NotifyDelegate);

		FScriptDelegate EndDelegate;
		EndDelegate.BindUFunction(this, "ReloadAnimationEndCallback");
		OwningCharacter->Mesh1P->GetAnimInstance()->OnMontageEnded.AddUnique(EndDelegate);
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
	}
}

void ABaseWeapon::ReloadAnimationEndCallback(UAnimMontage* Montage, bool bInterrupted) {
	if(Montage == ReloadAnimation) {
		CurrentState = EWeaponState::IDLE;
	}
}

