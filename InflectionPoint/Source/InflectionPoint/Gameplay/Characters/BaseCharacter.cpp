// Copyright 1998-2016 Epic Games, Inc. All Rights Reserved.

#include "InflectionPoint.h"
#include "BaseCharacter.h"
#include "Gameplay/Weapons/InflectionPointProjectile.h"
#include "Animation/AnimInstance.h"
#include "GameFramework/InputSettings.h"
#include "Kismet/HeadMountedDisplayFunctionLibrary.h"
#include "MotionControllerComponent.h"
#include "Gameplay/Recording/InputRecorder.h"
#include "Gameplay/Damage/MortalityProvider.h"
#include "Utils/CheckFunctions.h"

DEFINE_LOG_CATEGORY_STATIC(LogFPChar, Warning, All);

//////////////////////////////////////////////////////////////////////////
// ABaseCharacter

ABaseCharacter::ABaseCharacter() {
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(55.f, 96.0f);

	// Create a CameraComponent	
	FirstPersonCameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("FirstPersonCamera"));
	FirstPersonCameraComponent->SetupAttachment(GetCapsuleComponent());
	FirstPersonCameraComponent->RelativeLocation = FVector(-39.56f, 1.75f, 64.f); // Position the camera
	FirstPersonCameraComponent->bUsePawnControlRotation = true;

	// Create a mesh component that will be used when being viewed from a '1st person' view (when controlling this pawn)
	Mesh1P = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("CharacterMesh1P"));
	Mesh1P->SetOnlyOwnerSee(true);
	Mesh1P->SetupAttachment(FirstPersonCameraComponent);
	Mesh1P->bCastDynamicShadow = false;
	Mesh1P->CastShadow = false;
	Mesh1P->RelativeRotation = FRotator(1.9f, -19.19f, 5.2f);
	Mesh1P->RelativeLocation = FVector(-0.5f, -4.4f, -155.7f);

	// Create a gun mesh component
	FP_Gun = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("FP_Gun"));
	FP_Gun->SetOnlyOwnerSee(true);			// only the owning player will see this mesh
	FP_Gun->bCastDynamicShadow = false;
	FP_Gun->CastShadow = false;
	// FP_Gun->SetupAttachment(Mesh1P, TEXT("GripPoint"));
	FP_Gun->SetupAttachment(GetCapsuleComponent());

	FP_MuzzleLocation = CreateDefaultSubobject<USceneComponent>(TEXT("MuzzleLocation"));
	FP_MuzzleLocation->SetupAttachment(FP_Gun);
	/*FP_MuzzleLocation->SetRelativeLocation(FVector(0.2f, 48.4f, -10.6f));*/
	FP_MuzzleLocation->SetRelativeLocation(FVector(0.2f, 74.f, 11.f));

	// Default offset from the character location for projectiles to spawn
	GunOffset = FVector(100.0f, 0.0f, 10.0f);

	// Note: The ProjectileClass and the skeletal mesh/anim blueprints for Mesh1P, FP_Gun, and VR_Gun 
	// are set in the derived blueprint asset named MyCharacter to avoid direct content references in C++.

	// Mortality Provider
	MortalityProvider = CreateDefaultSubobject<UMortalityProvider>(TEXT("MortalityProvider"));
	MortalityProvider->SetIsReplicated(true);
}

void ABaseCharacter::BeginPlay() {
	// Call the base class  
	Super::BeginPlay();

	//Attach gun mesh component to Skeleton, doing it here because the skeleton is not yet created in the constructor
	FP_Gun->AttachToComponent(Mesh1P, FAttachmentTransformRules(EAttachmentRule::SnapToTarget, true), TEXT("GripPoint"));

	// Show or hide the two versions of the gun based on whether or not we're using motion controllers.
	Mesh1P->SetHiddenInGame(false, true);

}

float ABaseCharacter::TakeDamage(float DamageAmount, FDamageEvent const & DamageEvent, AController * EventInstigator, AActor * DamageCauser) {
	const float ActualDamage = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);
	//MortalityProvider->TakeDamage(ActualDamage, DamageEvent, EventInstigator, DamageCauser);
	MortalityProvider->TakeDamage(ActualDamage);
	return ActualDamage;
}

void ABaseCharacter::OnFire() {
	ServerFireProjectile(ProjectileClass);
}

void ABaseCharacter::OnDebugFire() {
	ServerFireProjectile(DebugProjectileClass);
}

bool ABaseCharacter::ServerFireProjectile_Validate(TSubclassOf<class AInflectionPointProjectile> projectileClassToSpawn) {
	return true;
}

void ABaseCharacter::ServerFireProjectile_Implementation(TSubclassOf<class AInflectionPointProjectile> projectileClassToSpawn) {
	//TakeDamage(25, FPointDamageEvent(), NULL, NULL);

	// try and fire a projectile
	if(projectileClassToSpawn != NULL) {
		UWorld* const World = GetWorld();
		if(AssertNotNull(World, GetWorld(), __FILE__, __LINE__)) {
			//const FRotator SpawnRotation = GetControlRotation();
			const FRotator SpawnRotation = FirstPersonCameraComponent->GetComponentRotation();

			// MuzzleOffset is in camera space, so transform it to world space before offsetting from the character location to find the final muzzle position
			const FVector SpawnLocation = ((FP_MuzzleLocation != nullptr) ? FP_MuzzleLocation->GetComponentLocation() : GetActorLocation());// +SpawnRotation.RotateVector(GunOffset);

			//Set Spawn Collision Handling Override
			FActorSpawnParameters ActorSpawnParams;
			ActorSpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButDontSpawnIfColliding;
			ActorSpawnParams.Instigator = Instigator;
			ActorSpawnParams.Owner = this;

			// spawn the projectile at the muzzle
			AInflectionPointProjectile* projectile = World->SpawnActor<AInflectionPointProjectile>(projectileClassToSpawn, SpawnLocation, SpawnRotation, ActorSpawnParams);

			MulticastProjectileFired();
		}
	}

}

void ABaseCharacter::MulticastProjectileFired_Implementation() {
	// try and play the sound if specified
	if(FireSound != NULL) {
		UGameplayStatics::SpawnSoundAttached(FireSound, GetCapsuleComponent());
	}

	// try and play a firing animation if specified
	if(FireAnimation != NULL) {
		// Get the animation object for the arms mesh
		UAnimInstance* AnimInstance = Mesh1P->GetAnimInstance();
		if(AnimInstance != NULL) {
			AnimInstance->Montage_Play(FireAnimation, 1.f);
		}
	}
}

void ABaseCharacter::MoveForward(float value) {
	if(value != 0.0f) {
		AddMovementInput(GetActorForwardVector(), value);
	}
}

void ABaseCharacter::MoveRight(float value) {
	if(value != 0.0f) {
		AddMovementInput(GetActorRightVector(), value);
	}
}

void ABaseCharacter::TurnAtRate(float rate) {
	// calculate delta for this frame from the rate information
	AddControllerYawInput(rate * BaseTurnRate * GetWorld()->GetDeltaSeconds());
}

void ABaseCharacter::LookUpAtRate(float rate) {
	// calculate delta for this frame from the rate information
	AddControllerPitchInput(rate * BaseLookUpRate * GetWorld()->GetDeltaSeconds());
}
