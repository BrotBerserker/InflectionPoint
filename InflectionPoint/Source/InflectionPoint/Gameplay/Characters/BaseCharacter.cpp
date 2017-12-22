// Copyright 1998-2016 Epic Games, Inc. All Rights Reserved.

#include "InflectionPoint.h"
#include "BaseCharacter.h"
#include "Gameplay/Weapons/InflectionPointProjectile.h"
#include "Animation/AnimInstance.h"
#include "GameFramework/InputSettings.h"
#include "Kismet/HeadMountedDisplayFunctionLibrary.h"
#include "MotionControllerComponent.h"
#include "Gameplay/Damage/MortalityProvider.h"
#include "Gameplay/Recording/PlayerStateRecorder.h"
#include "Utils/CheckFunctions.h"
#include "Gamemodes/TDMPlayerStateBase.h"
#include "Gamemodes/TDMGameStateBase.h"

DEFINE_LOG_CATEGORY_STATIC(LogFPChar, Warning, All);

//////////////////////////////////////////////////////////////////////////
// ABaseCharacter

ABaseCharacter::ABaseCharacter() {
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(55.f, 96.0f);

	// Create a CameraComponent	
	FirstPersonCameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("FirstPersonCamera"));
	FirstPersonCameraComponent->SetupAttachment(GetCapsuleComponent());
	//FirstPersonCameraComponent->RelativeLocation = FVector(-39.56f, 1.75f, 64.f); 
	FirstPersonCameraComponent->RelativeLocation = FVector(-20.5f, 1.75f, 41.f); // Position the camera
	FirstPersonCameraComponent->RelativeScale3D = FVector(0.4, 0.4, 0.4); // Scale of the camera
	FirstPersonCameraComponent->bUsePawnControlRotation = true;

	// Create a mesh component that will be used when being viewed from a '1st person' view (when controlling this pawn)
	Mesh1P = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("CharacterMesh1P"));
	Mesh1P->SetOnlyOwnerSee(true);
	Mesh1P->SetupAttachment(FirstPersonCameraComponent);
	Mesh1P->bCastDynamicShadow = false;
	Mesh1P->CastShadow = false;
	Mesh1P->RelativeLocation = FVector(3.09f, 0.61f, -160.7f);
	Mesh1P->RelativeRotation = FRotator(1.9f, -19.19f, 5.2f);

	// Create a gun mesh component
	FP_Gun = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("FP_Gun"));
	FP_Gun->SetOnlyOwnerSee(true);			// only the owning player will see this mesh
	FP_Gun->bCastDynamicShadow = false;
	FP_Gun->CastShadow = false;
	// FP_Gun->SetupAttachment(Mesh1P, TEXT("GripPoint"));
	FP_Gun->SetupAttachment(GetCapsuleComponent());
	FP_Gun->RelativeScale3D = FVector(.4, .4, .4);

	// MuzzleLocation, where shots will be spawned
	FP_MuzzleLocation = CreateDefaultSubobject<USceneComponent>(TEXT("MuzzleLocation"));
	FP_MuzzleLocation->SetupAttachment(FP_Gun);
	/*FP_MuzzleLocation->SetRelativeLocation(FVector(0.2f, 48.4f, -10.6f));*/
	//FP_MuzzleLocation->SetRelativeLocation(FVector(0.2f, 172.f, 11.f));
	FP_MuzzleLocation->SetRelativeLocation(FVector(0.2f, 60.f, 11.f));

	// Create the '3rd person' body mesh
	Mesh3P = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("CharacterMesh3P"));
	Mesh3P->SetupAttachment(GetCapsuleComponent());
	Mesh3P->SetOwnerNoSee(true);
	Mesh3P->RelativeLocation = FVector(0.f, 0.f, -97.f);
	Mesh3P->RelativeRotation = FRotator(0.f, -90.f, 0.f);

	// Create the '3rd person' gun mesh
	TP_Gun = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("TP_Gun"));
	TP_Gun->SetOwnerNoSee(true);
	//TP_Gun->SetupAttachment(Mesh3P, TEXT("GripPoint"));
	TP_Gun->SetupAttachment(GetCapsuleComponent());

	// Initialize MortalityProvider
	MortalityProvider = CreateDefaultSubobject<UMortalityProvider>(TEXT("MortalityProvider"));
	MortalityProvider->SetIsReplicated(true);
}

void ABaseCharacter::BeginPlay() {
	// Call the base class  
	Super::BeginPlay();

	//Attach gun mesh component to Skeleton, doing it here because the skeleton is not yet created in the constructor
	FP_Gun->AttachToComponent(Mesh1P, FAttachmentTransformRules(EAttachmentRule::SnapToTarget, true), TEXT("GripPoint"));

	TP_Gun->AttachToComponent(Mesh3P, FAttachmentTransformRules(EAttachmentRule::SnapToTarget, true), TEXT("GripPoint"));

	// Detaches MuzzleLocation from weapon to prevent the weapon animation from moving the MuzzleLocation
	FP_MuzzleLocation->AttachToComponent(FirstPersonCameraComponent, FAttachmentTransformRules(EAttachmentRule::KeepWorld, true));

	// Show or hide the two versions of the gun based on whether or not we're using motion controllers.
	Mesh1P->SetHiddenInGame(false, true);
}

void ABaseCharacter::Restart() {
	Super::Restart();
	OnRestart();
}

void ABaseCharacter::Tick(float DeltaTime) {
	Super::Tick(DeltaTime);

	if(!initialized && IsReadyForInitialization()) {
		Initialize();
		OnInitialized();
		initialized = true;
	} 
}

void ABaseCharacter::ApplyPlayerColor(ATDMPlayerStateBase* playerState) {
	ATDMGameStateBase* gameState = Cast<ATDMGameStateBase>(UGameplayStatics::GetGameState(GetWorld()));
	AssertNotNull(gameState, GetWorld(), __FILE__, __LINE__, TEXT("GameState is null!"));

	UMaterialInstanceDynamic* dynamicMaterial = UMaterialInstanceDynamic::Create(Mesh3P->GetMaterial(0), Mesh3P);
	dynamicMaterial->SetVectorParameterValue("BodyColor", gameState->TeamColors[playerState->Team]);
	Mesh3P->SetMaterial(0, dynamicMaterial);
	Mesh1P->SetMaterial(0, dynamicMaterial);
}

void ABaseCharacter::MulticastApplyPlayerColor_Implementation(ATDMPlayerStateBase* playerState) {
	ApplyPlayerColor(playerState);
}

float ABaseCharacter::TakeDamage(float DamageAmount, FDamageEvent const & DamageEvent, AController * EventInstigator, AActor * DamageCauser) {
	const float actualDamage = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);
	MortalityProvider->TakeDamage(actualDamage, EventInstigator, DamageCauser);
	if(EventInstigator) {
		FVector directionVector = (EventInstigator->GetCharacter()->GetActorLocation() - GetActorLocation());
		directionVector.Normalize();
		OnDirectionalDamageReceived(directionVector, actualDamage);
	}
	return actualDamage;
}

void ABaseCharacter::OnFire() {
	ServerFireProjectile(ProjectileClass);
}

void ABaseCharacter::OnDebugFire() {
	ServerFireProjectile(DebugProjectileClass);
}

void ABaseCharacter::OnStopFire() {
	ServerStopFire();
}

bool ABaseCharacter::ServerFireProjectile_Validate(TSubclassOf<class AInflectionPointProjectile> projectileClassToSpawn) {
	return true;
}

void ABaseCharacter::ServerFireProjectile_Implementation(TSubclassOf<class AInflectionPointProjectile> projectileClassToSpawn) {
	DrawDebugArrow();
	if(UGameplayStatics::GetRealTimeSeconds(GetWorld()) - LastShotTimeStamp >= DelayBetweenShots) {
		FireProjectile(projectileClassToSpawn);
		LastShotTimeStamp = UGameplayStatics::GetRealTimeSeconds(GetWorld());
	}
}

bool ABaseCharacter::ServerStopFire_Validate() {
	return true;
}

void ABaseCharacter::ServerStopFire_Implementation() {
	StopFire();
}

void ABaseCharacter::FireProjectile(TSubclassOf<AInflectionPointProjectile> &projectileClassToSpawn) {
	// try and fire a projectile
	if(projectileClassToSpawn != NULL) {
		UWorld* const World = GetWorld();
		if(AssertNotNull(World, GetWorld(), __FILE__, __LINE__)) {
			//Set Spawn Collision Handling Override
			FActorSpawnParameters ActorSpawnParams;
			ActorSpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
			ActorSpawnParams.Instigator = Instigator;
			ActorSpawnParams.Owner = this;

			// spawn the projectile at the muzzle
			AInflectionPointProjectile* projectile = World->SpawnActor<AInflectionPointProjectile>(projectileClassToSpawn, GetProjectileSpawnLocation(), GetProjectileSpawnRotation(), ActorSpawnParams);

			MulticastProjectileFired();
		}
	}
}

void ABaseCharacter::StopFire() {
	// do nothing
}

void ABaseCharacter::DrawDebugArrow() {
	if(DrawDebugArrows) {
		FRotator cameraRot = FirstPersonCameraComponent->GetComponentRotation();
		FVector cameraDirectionVector = cameraRot.Vector() * 15 + GetTransform().GetLocation();

		//DebugArrowColor.B
		DrawDebugDirectionalArrow(GetWorld(), GetTransform().GetLocation(), cameraDirectionVector, DebugArrowColor.B, DebugArrowColor, true, -1, 0, 0.5f);
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

void ABaseCharacter::MulticastOnDeath_Implementation() {
	// Play random Death Animation
	if(DeathAnimations.Num() == 0) {
		UE_LOG(LogTemp, Warning, TEXT("Warning: No Death Animations set for this character!"));
	} else {
		int index = FMath::RandRange(0, DeathAnimations.Num() - 1);
		Mesh3P->PlayAnimation(DeathAnimations[index], false);
	}

	// Disable all input
	DisableInput((APlayerController*)GetController());

	// Disable all collisions except for WorldStatic
	GetCapsuleComponent()->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECollisionChannel::ECC_WorldStatic, ECollisionResponse::ECR_Block);
}

void ABaseCharacter::ClientOnDeath_Implementation() {
	Mesh3P->SetOwnerNoSee(false);
	Mesh1P->SetVisibility(false, true);
	FirstPersonCameraComponent->SetRelativeLocation(FVector(-200.f, 1.75f, 130.f));
	AssertNotNull(GetController(), GetWorld(), __FILE__, __LINE__);
	FRotator rot = GetController()->GetControlRotation();
	GetController()->SetControlRotation(FRotator(-30, rot.Yaw, rot.Roll));
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
	AddControllerYawInput(rate);
}

void ABaseCharacter::LookUpAtRate(float rate) {
	// calculate delta for this frame from the rate information
	AddControllerPitchInput(rate);
	ServerUpdateCameraPitch(FirstPersonCameraComponent->GetComponentRotation().Pitch);
	MulticastUpdateCameraPitch(FirstPersonCameraComponent->GetComponentRotation().Pitch);
}

FRotator ABaseCharacter::GetProjectileSpawnRotation() {
	return FirstPersonCameraComponent->GetComponentRotation();
}

FVector ABaseCharacter::GetProjectileSpawnLocation() {
	return ((FP_MuzzleLocation != nullptr) ? FP_MuzzleLocation->GetComponentLocation() : GetActorLocation());
}

bool ABaseCharacter::ServerUpdateCameraPitch_Validate(float pitch) {
	return true;
}

void ABaseCharacter::ServerUpdateCameraPitch_Implementation(float pitch) {
	auto currentRot = FirstPersonCameraComponent->GetComponentRotation();
	currentRot.Pitch = pitch;
	currentRot.Roll = 0;
	currentRot.Yaw = 0;
	FirstPersonCameraComponent->SetRelativeRotation(currentRot);
}

void ABaseCharacter::MulticastUpdateCameraPitch_Implementation(float pitch) {
	auto currentRot = FirstPersonCameraComponent->GetComponentRotation();
	currentRot.Pitch = pitch;
	currentRot.Roll = 0;
	currentRot.Yaw = 0;
	FirstPersonCameraComponent->SetRelativeRotation(currentRot);
}