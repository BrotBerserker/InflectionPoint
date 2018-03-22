// Copyright 1998-2016 Epic Games, Inc. All Rights Reserved.

#include "InflectionPoint.h"
#include "BaseCharacter.h"
#include "Gameplay/Weapons/InflectionPointProjectile.h"
#include "Gameplay/Weapons/WeaponInventory.h"
#include "Animation/AnimInstance.h"
#include "GameFramework/InputSettings.h"
#include "Kismet/HeadMountedDisplayFunctionLibrary.h"
#include "MotionControllerComponent.h"
#include "Gameplay/Damage/MortalityProvider.h"
#include "Gameplay/Recording/PlayerStateRecorder.h"
#include "Utils/CheckFunctions.h"
#include "Gamemodes/TDMPlayerStateBase.h"
#include "DebugTools/InflectionPointCheatManager.h"
#include "Gamemodes/TDMGameStateBase.h"

DEFINE_LOG_CATEGORY_STATIC(LogFPChar, Warning, All);

void ABaseCharacter::GetLifetimeReplicatedProps(TArray< FLifetimeProperty > & OutLifetimeProps) const {
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ABaseCharacter, CurrentWeapon);
}

//////////////////////////////////////////////////////////////////////////
// ABaseCharacter

ABaseCharacter::ABaseCharacter() {
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(55.f, 96.0f);

	// Create a CameraComponent	
	FirstPersonCameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("FirstPersonCamera"));
	FirstPersonCameraComponent->SetupAttachment(GetCapsuleComponent());
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

	// Create the '3rd person' body mesh
	Mesh3P = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("CharacterMesh3P"));
	Mesh3P->SetupAttachment(GetCapsuleComponent());
	Mesh3P->SetOwnerNoSee(true);
	Mesh3P->RelativeLocation = FVector(0.f, 0.f, -97.f);
	Mesh3P->RelativeRotation = FRotator(0.f, -90.f, 0.f);

	WeaponInventory = CreateDefaultSubobject<UWeaponInventory>(TEXT("WeaponInventory"));

	// Initialize MortalityProvider
	MortalityProvider = CreateDefaultSubobject<UMortalityProvider>(TEXT("MortalityProvider"));
	MortalityProvider->SetIsReplicated(true);

	// Initialize Materialize Timeline (wtf aber ok, siehe https://wiki.unrealengine.com/Timeline_in_c%2B%2B)
	MaterializeTimeline = CreateDefaultSubobject<UTimelineComponent>(TEXT("MaterializeTimeline"));

	walkSpeed = GetCharacterMovement()->MaxWalkSpeed;

	CharacterInfoProvider = CreateDefaultSubobject<UCharacterInfoProvider>(TEXT("CharacterInfoProvider"));
	CharacterInfoProvider->SetIsReplicated(true);
}

void ABaseCharacter::BeginPlay() {
	// Call the base class  
	Super::BeginPlay();

	if(HasAuthority()) {
		EquipWeapon(WeaponInventory->GetNextWeapon(NULL));
	}

	// Show or hide the two versions of the gun based on whether or not we're using motion controllers.
	Mesh1P->SetHiddenInGame(false, true);

	// Create dynamic materials for materialize animation
	DynamicBodyMaterial = UMaterialInstanceDynamic::Create(Mesh3P->GetMaterial(0), Mesh3P);
	Mesh3P->SetMaterial(0, DynamicBodyMaterial);
	Mesh1P->SetMaterial(0, DynamicBodyMaterial);
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

	UpdateFieldOfView(DeltaTime);
}

void ABaseCharacter::Destroyed() {
	Super::Destroyed();
	WeaponInventory->Destroy();
}

void ABaseCharacter::UpdateFieldOfView(float DeltaTime) {
	float targetFoV = IsAiming ? 75.f : 90.f;
	FirstPersonCameraComponent->SetFieldOfView(FMath::FInterpTo(FirstPersonCameraComponent->FieldOfView, targetFoV, DeltaTime, 14.f));
}

void ABaseCharacter::ApplyPlayerColor(ATDMPlayerStateBase* playerState) {
	ATDMGameStateBase* gameState = Cast<ATDMGameStateBase>(UGameplayStatics::GetGameState(GetWorld()));
	AssertNotNull(gameState, GetWorld(), __FILE__, __LINE__, TEXT("GameState is null!"));

	DynamicBodyMaterial->SetVectorParameterValue("BodyColor", gameState->TeamColors[playerState->Team]);
}

void ABaseCharacter::MulticastApplyPlayerColor_Implementation(ATDMPlayerStateBase* playerState) {
	ApplyPlayerColor(playerState);
}

void ABaseCharacter::ShowSpawnAnimation() {
	AssertNotNull(MaterializeCurve, GetWorld(), __FILE__, __LINE__, TEXT("No materialize curve has been set!"));

	// add curve to timeline
	FOnTimelineFloat callback{};
	callback.BindUFunction(this, FName{ TEXT("MaterializeCallback") });
	MaterializeTimeline->AddInterpFloat(MaterializeCurve, callback, FName{ TEXT("MaterializeTimelineAnimation") });

	// set timeline finish callback
	FOnTimelineEvent finishCallback{};
	finishCallback.BindUFunction(this, FName{ TEXT("MaterializeFinishCallback") });
	MaterializeTimeline->SetTimelineFinishedFunc(finishCallback);

	MaterializeTimeline->Play();
}

void ABaseCharacter::MaterializeCallback(float value) {
	DynamicBodyMaterial->SetScalarParameterValue("Materialize Amount", value);
}

void ABaseCharacter::MaterializeFinishCallback() {
	// Switch to materials without materialize effect to save a lot of performance
	UMaterialInstanceDynamic* dynamicMaterialWithoutMaterialize = UMaterialInstanceDynamic::Create(BodyMaterialAfterMaterialize, Mesh3P);
	FLinearColor bodyColor;
	DynamicBodyMaterial->GetVectorParameterValue("BodyColor", bodyColor);
	dynamicMaterialWithoutMaterialize->SetVectorParameterValue("BodyColor", bodyColor);
	Mesh3P->SetMaterial(0, dynamicMaterialWithoutMaterialize);
	Mesh1P->SetMaterial(0, dynamicMaterialWithoutMaterialize);
}

void ABaseCharacter::MulticastShowSpawnAnimation_Implementation() {
	ShowSpawnAnimation();
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

void ABaseCharacter::StartFire() {
	DisableSprint();
	sprintAllowed = false;
	ServerStartFire();
}

bool ABaseCharacter::ServerStartFire_Validate() {
	return true;
}

void ABaseCharacter::ServerStartFire_Implementation() {
	DrawDebugArrow();
	if(!AssertNotNull(CurrentWeapon, GetWorld(), __FILE__, __LINE__))
		return;
	CurrentWeapon->StartFire();
}

void ABaseCharacter::StopFire() {
	sprintAllowed = true;
	ServerStopFire();
}

bool ABaseCharacter::ServerStopFire_Validate() {
	return true;
}

void ABaseCharacter::ServerStopFire_Implementation() {
	if(!AssertNotNull(CurrentWeapon, GetWorld(), __FILE__, __LINE__))
		return;
	CurrentWeapon->StopFire();
}

void ABaseCharacter::StartAiming() {
	IsAiming = true;
	if(!HasAuthority()) {
		ServerStartAiming();
	}
}

bool ABaseCharacter::ServerStartAiming_Validate() {
	return true;
}

void ABaseCharacter::ServerStartAiming_Implementation() {
	IsAiming = true;
	MulticastStartAiming();
}

void ABaseCharacter::MulticastStartAiming_Implementation() {
	IsAiming = true;
}

void ABaseCharacter::StopAiming() {
	IsAiming = false;
	if(!HasAuthority()) {
		ServerStopAiming();
	}
}

bool ABaseCharacter::ServerStopAiming_Validate() {
	return true;
}

void ABaseCharacter::ServerStopAiming_Implementation() {
	IsAiming = false;
	MulticastStopAiming();
}

void ABaseCharacter::MulticastStopAiming_Implementation() {
	IsAiming = false;
}

bool ABaseCharacter::ServerReload_Validate() {
	return true;
}

void ABaseCharacter::ServerReload_Implementation() {
	if(!AssertNotNull(CurrentWeapon, GetWorld(), __FILE__, __LINE__))
		return;
	CurrentWeapon->Reload();
}

bool ABaseCharacter::ServerEquipNextWeapon_Validate() {
	return true;
}

void ABaseCharacter::ServerEquipNextWeapon_Implementation() {
	EquipWeapon(WeaponInventory->GetNextWeapon(CurrentWeapon), CurrentWeapon);
}

bool ABaseCharacter::ServerEquipPreviousWeapon_Validate() {
	return true;
}

void ABaseCharacter::ServerEquipPreviousWeapon_Implementation() {
	EquipWeapon(WeaponInventory->GetPreviousWeapon(CurrentWeapon), CurrentWeapon);
}


bool ABaseCharacter::ServerEquipSpecificWeapon_Validate(int index) {
	return true;
}

void ABaseCharacter::ServerEquipSpecificWeapon_Implementation(int index) {
	ABaseWeapon* newWeapon = WeaponInventory->GetWeapon(index);
	if(newWeapon && CurrentWeapon != newWeapon)
		EquipWeapon(newWeapon, CurrentWeapon);
}

void ABaseCharacter::OnRep_CurrentWeapon(ABaseWeapon* OldWeapon) {
	EquipWeapon(CurrentWeapon, OldWeapon);
}

void ABaseCharacter::EquipWeapon(ABaseWeapon* NewWeapon, ABaseWeapon* OldWeapon) {
	CurrentWeapon = NewWeapon;

	if(OldWeapon) {
		OldWeapon->OnUnequip();
	}

	CurrentWeapon->OnEquip();
}

void ABaseCharacter::DrawDebugArrow() {
	if(Cast<UInflectionPointCheatManager>(GetWorld()->GetFirstPlayerController()->CheatManager) &&
		Cast<UInflectionPointCheatManager>(GetWorld()->GetFirstPlayerController()->CheatManager)->IsCharacterDebugArrowsEnabled) {
		FRotator cameraRot = FirstPersonCameraComponent->GetComponentRotation();
		FVector cameraDirectionVector = cameraRot.Vector() * 15 + GetTransform().GetLocation();

		//DebugArrowColor.B
		DrawDebugDirectionalArrow(GetWorld(), GetTransform().GetLocation(), cameraDirectionVector, DebugArrowColor.B, DebugArrowColor, true, -1, 0, 0.5f);
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
	DisableComponentsSimulatePhysics();

	GetCapsuleComponent()->SetCollisionProfileName(FName("DeadCharacter"));
}

void ABaseCharacter::ClientOnDeath_Implementation() {
	Mesh3P->SetOwnerNoSee(false);
	Mesh1P->SetVisibility(false, true);
	FirstPersonCameraComponent->SetRelativeLocation(FVector(-200.f, 1.75f, 130.f));
	if(!AssertNotNull(GetController(), GetWorld(), __FILE__, __LINE__))
		return;
	FRotator rot = GetController()->GetControlRotation();
	GetController()->SetControlRotation(FRotator(-30, rot.Yaw, rot.Roll));
}

void ABaseCharacter::MoveForward(float value) {
	if(value != 0.0f) {
		AddMovementInput(GetActorForwardVector(), value);
	}

	if(ShouldStartSprinting(value)) {
		StartSprinting();
		ServerStartSprinting();
	} else if(ShouldStopSprinting(value)) {
		StopSprinting();
		ServerStopSprinting();
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

void ABaseCharacter::EnableSprint() {
	sprintEnabled = true;
}

void ABaseCharacter::DisableSprint() {
	sprintEnabled = false;
}

bool ABaseCharacter::ShouldStartSprinting(float ForwardMovement) {
	return sprintAllowed && sprintEnabled && ForwardMovement > 0 && GetVelocity().Size() <= walkSpeed;
}

bool ABaseCharacter::ShouldStopSprinting(float ForwardMovement) {
	if(GetVelocity().Size() <= walkSpeed) {
		return false;
	}
	return !sprintAllowed || !sprintEnabled || ForwardMovement <= 0;
}

void ABaseCharacter::StartSprinting() {
	GetCharacterMovement()->MaxWalkSpeed = SprintSpeed;
}

void ABaseCharacter::StopSprinting() {
	GetCharacterMovement()->MaxWalkSpeed = walkSpeed;
}

bool ABaseCharacter::ServerStartSprinting_Validate() {
	return true;
}

void ABaseCharacter::ServerStartSprinting_Implementation() {
	StartSprinting();
}

bool ABaseCharacter::ServerStopSprinting_Validate() {
	return true;
}

void ABaseCharacter::ServerStopSprinting_Implementation() {
	StopSprinting();
}

