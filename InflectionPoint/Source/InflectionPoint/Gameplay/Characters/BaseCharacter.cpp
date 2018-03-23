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

void ABaseCharacter::GetLifetimeReplicatedProps(TArray< FLifetimeProperty > & OutLifetimeProps) const {DebugPrint(__FILE__, __LINE__);
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ABaseCharacter, CurrentWeapon);
DebugPrint(__FILE__, __LINE__);}

//////////////////////////////////////////////////////////////////////////
// ABaseCharacter

ABaseCharacter::ABaseCharacter() {DebugPrint(__FILE__, __LINE__);
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
DebugPrint(__FILE__, __LINE__);}

void ABaseCharacter::BeginPlay() {DebugPrint(__FILE__, __LINE__);
	// Call the base class  
	Super::BeginPlay();

	if(HasAuthority()) {DebugPrint(__FILE__, __LINE__);
		EquipWeapon(WeaponInventory->GetNextWeapon(NULL));
	DebugPrint(__FILE__, __LINE__);}

	// Show or hide the two versions of the gun based on whether or not we're using motion controllers.
	Mesh1P->SetHiddenInGame(false, true);

	// Create dynamic materials for materialize animation
	DynamicBodyMaterial = UMaterialInstanceDynamic::Create(Mesh3P->GetMaterial(0), Mesh3P);
	Mesh3P->SetMaterial(0, DynamicBodyMaterial);
	Mesh1P->SetMaterial(0, DynamicBodyMaterial);
DebugPrint(__FILE__, __LINE__);}

void ABaseCharacter::Restart() {DebugPrint(__FILE__, __LINE__);
	Super::Restart();
	OnRestart();
DebugPrint(__FILE__, __LINE__);}

void ABaseCharacter::Tick(float DeltaTime) {DebugPrint(__FILE__, __LINE__);
	Super::Tick(DeltaTime);

	if(!initialized && IsReadyForInitialization()) {DebugPrint(__FILE__, __LINE__);
		Initialize();
		OnInitialized();
		initialized = true;
	DebugPrint(__FILE__, __LINE__);}

	UpdateFieldOfView(DeltaTime);
DebugPrint(__FILE__, __LINE__);}

void ABaseCharacter::Destroyed() {DebugPrint(__FILE__, __LINE__);
	Super::Destroyed();
	WeaponInventory->Destroy();
DebugPrint(__FILE__, __LINE__);}

void ABaseCharacter::UpdateFieldOfView(float DeltaTime) {DebugPrint(__FILE__, __LINE__);
	float targetFoV = IsAiming ? 75.f : 90.f;
	FirstPersonCameraComponent->SetFieldOfView(FMath::FInterpTo(FirstPersonCameraComponent->FieldOfView, targetFoV, DeltaTime, 14.f));
DebugPrint(__FILE__, __LINE__);}

void ABaseCharacter::ApplyPlayerColor(ATDMPlayerStateBase* playerState) {DebugPrint(__FILE__, __LINE__);
	ATDMGameStateBase* gameState = Cast<ATDMGameStateBase>(UGameplayStatics::GetGameState(GetWorld()));
	AssertNotNull(gameState, GetWorld(), __FILE__, __LINE__, TEXT("GameState is null!"));

	DynamicBodyMaterial->SetVectorParameterValue("BodyColor", gameState->TeamColors[playerState->Team]);
DebugPrint(__FILE__, __LINE__);}

void ABaseCharacter::MulticastApplyPlayerColor_Implementation(ATDMPlayerStateBase* playerState) {DebugPrint(__FILE__, __LINE__);
	ApplyPlayerColor(playerState);
DebugPrint(__FILE__, __LINE__);}

void ABaseCharacter::ShowSpawnAnimation() {DebugPrint(__FILE__, __LINE__);
	AssertNotNull(MaterializeCurve, GetWorld(), __FILE__, __LINE__, TEXT("No materialize curve has been set!"));

	// add curve to timeline
	FOnTimelineFloat callback{};
	callback.BindUFunction(this, FName{TEXT("MaterializeCallback")});
	MaterializeTimeline->AddInterpFloat(MaterializeCurve, callback, FName{TEXT("MaterializeTimelineAnimation") });

	// set timeline finish callback
	FOnTimelineEvent finishCallback{};
	finishCallback.BindUFunction(this, FName{TEXT("MaterializeFinishCallback")});
	MaterializeTimeline->SetTimelineFinishedFunc(finishCallback);

	MaterializeTimeline->Play();
DebugPrint(__FILE__, __LINE__);}

void ABaseCharacter::MaterializeCallback(float value) {DebugPrint(__FILE__, __LINE__);
	DynamicBodyMaterial->SetScalarParameterValue("Materialize Amount", value);
DebugPrint(__FILE__, __LINE__);}

void ABaseCharacter::MaterializeFinishCallback() {DebugPrint(__FILE__, __LINE__);
	// Switch to materials without materialize effect to save a lot of performance
	UMaterialInstanceDynamic* dynamicMaterialWithoutMaterialize = UMaterialInstanceDynamic::Create(BodyMaterialAfterMaterialize, Mesh3P);
	FLinearColor bodyColor;
	DynamicBodyMaterial->GetVectorParameterValue("BodyColor", bodyColor);
	dynamicMaterialWithoutMaterialize->SetVectorParameterValue("BodyColor", bodyColor);
	Mesh3P->SetMaterial(0, dynamicMaterialWithoutMaterialize);
	Mesh1P->SetMaterial(0, dynamicMaterialWithoutMaterialize);
DebugPrint(__FILE__, __LINE__);}

void ABaseCharacter::MulticastShowSpawnAnimation_Implementation() {DebugPrint(__FILE__, __LINE__);
	ShowSpawnAnimation();
DebugPrint(__FILE__, __LINE__);}

float ABaseCharacter::TakeDamage(float DamageAmount, FDamageEvent const & DamageEvent, AController * EventInstigator, AActor * DamageCauser) {DebugPrint(__FILE__, __LINE__);
	const float actualDamage = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);
	MortalityProvider->TakeDamage(actualDamage, EventInstigator, DamageCauser);
	if(EventInstigator) {DebugPrint(__FILE__, __LINE__);
		FVector directionVector = (EventInstigator->GetCharacter()->GetActorLocation() - GetActorLocation());
		directionVector.Normalize();
		OnDirectionalDamageReceived(directionVector, actualDamage);
	DebugPrint(__FILE__, __LINE__);}
	return actualDamage;
DebugPrint(__FILE__, __LINE__);}

void ABaseCharacter::StartFire() {DebugPrint(__FILE__, __LINE__);
	DisableSprint();
	sprintAllowed = false;
	ServerStartFire();
DebugPrint(__FILE__, __LINE__);}

bool ABaseCharacter::ServerStartFire_Validate() {DebugPrint(__FILE__, __LINE__);
	return true;
DebugPrint(__FILE__, __LINE__);}

void ABaseCharacter::ServerStartFire_Implementation() {DebugPrint(__FILE__, __LINE__);
	DrawDebugArrow();
	if(!AssertNotNull(CurrentWeapon, GetWorld(), __FILE__, __LINE__))
		return;
	CurrentWeapon->StartFire();
DebugPrint(__FILE__, __LINE__);}

void ABaseCharacter::StopFire() {DebugPrint(__FILE__, __LINE__);
	sprintAllowed = true;
	ServerStopFire();
DebugPrint(__FILE__, __LINE__);}

bool ABaseCharacter::ServerStopFire_Validate() {DebugPrint(__FILE__, __LINE__);
	return true;
DebugPrint(__FILE__, __LINE__);}

void ABaseCharacter::ServerStopFire_Implementation() {DebugPrint(__FILE__, __LINE__);
	if(!AssertNotNull(CurrentWeapon, GetWorld(), __FILE__, __LINE__))
		return;
	CurrentWeapon->StopFire();
DebugPrint(__FILE__, __LINE__);}

void ABaseCharacter::StartAiming() {DebugPrint(__FILE__, __LINE__);
	IsAiming = true;
	if(!HasAuthority()) {DebugPrint(__FILE__, __LINE__);
		ServerStartAiming();
	DebugPrint(__FILE__, __LINE__);}
DebugPrint(__FILE__, __LINE__);}

bool ABaseCharacter::ServerStartAiming_Validate() {DebugPrint(__FILE__, __LINE__);
	return true;
DebugPrint(__FILE__, __LINE__);}

void ABaseCharacter::ServerStartAiming_Implementation() {DebugPrint(__FILE__, __LINE__);
	IsAiming = true;
	MulticastStartAiming();
DebugPrint(__FILE__, __LINE__);}

void ABaseCharacter::MulticastStartAiming_Implementation() {DebugPrint(__FILE__, __LINE__);
	IsAiming = true;
DebugPrint(__FILE__, __LINE__);}

void ABaseCharacter::StopAiming() {DebugPrint(__FILE__, __LINE__);
	IsAiming = false;
	if(!HasAuthority()) {DebugPrint(__FILE__, __LINE__);
		ServerStopAiming();
	DebugPrint(__FILE__, __LINE__);}
DebugPrint(__FILE__, __LINE__);}

bool ABaseCharacter::ServerStopAiming_Validate() {DebugPrint(__FILE__, __LINE__);
	return true;
DebugPrint(__FILE__, __LINE__);}

void ABaseCharacter::ServerStopAiming_Implementation() {DebugPrint(__FILE__, __LINE__);
	IsAiming = false;
	MulticastStopAiming();
DebugPrint(__FILE__, __LINE__);}

void ABaseCharacter::MulticastStopAiming_Implementation() {DebugPrint(__FILE__, __LINE__);
	IsAiming = false;
DebugPrint(__FILE__, __LINE__);}

bool ABaseCharacter::ServerReload_Validate() {DebugPrint(__FILE__, __LINE__);
	return true;
DebugPrint(__FILE__, __LINE__);}

void ABaseCharacter::ServerReload_Implementation() {DebugPrint(__FILE__, __LINE__);
	if(!AssertNotNull(CurrentWeapon, GetWorld(), __FILE__, __LINE__))
		return;
	CurrentWeapon->Reload();
DebugPrint(__FILE__, __LINE__);}

bool ABaseCharacter::ServerEquipNextWeapon_Validate() {DebugPrint(__FILE__, __LINE__);
	return true;
DebugPrint(__FILE__, __LINE__);}

void ABaseCharacter::ServerEquipNextWeapon_Implementation() {DebugPrint(__FILE__, __LINE__);
	EquipWeapon(WeaponInventory->GetNextWeapon(CurrentWeapon), CurrentWeapon);
DebugPrint(__FILE__, __LINE__);}

bool ABaseCharacter::ServerEquipPreviousWeapon_Validate() {DebugPrint(__FILE__, __LINE__);
	return true;
DebugPrint(__FILE__, __LINE__);}

void ABaseCharacter::ServerEquipPreviousWeapon_Implementation() {DebugPrint(__FILE__, __LINE__);
	EquipWeapon(WeaponInventory->GetPreviousWeapon(CurrentWeapon), CurrentWeapon);
DebugPrint(__FILE__, __LINE__);}


bool ABaseCharacter::ServerEquipSpecificWeapon_Validate(int index) {DebugPrint(__FILE__, __LINE__);
	return true;
DebugPrint(__FILE__, __LINE__);}

void ABaseCharacter::ServerEquipSpecificWeapon_Implementation(int index) {DebugPrint(__FILE__, __LINE__);
	ABaseWeapon* newWeapon = WeaponInventory->GetWeapon(index);
	if(newWeapon && CurrentWeapon != newWeapon)
		EquipWeapon(newWeapon, CurrentWeapon);
DebugPrint(__FILE__, __LINE__);}

void ABaseCharacter::OnRep_CurrentWeapon(ABaseWeapon* OldWeapon) {DebugPrint(__FILE__, __LINE__);
	EquipWeapon(CurrentWeapon, OldWeapon);
DebugPrint(__FILE__, __LINE__);}

void ABaseCharacter::EquipWeapon(ABaseWeapon* NewWeapon, ABaseWeapon* OldWeapon) {DebugPrint(__FILE__, __LINE__);
	CurrentWeapon = NewWeapon;

	if(OldWeapon) {DebugPrint(__FILE__, __LINE__);
		OldWeapon->OnUnequip();
	DebugPrint(__FILE__, __LINE__);}

	CurrentWeapon->OnEquip();
DebugPrint(__FILE__, __LINE__);}

void ABaseCharacter::DrawDebugArrow() {DebugPrint(__FILE__, __LINE__);
	if(Cast<UInflectionPointCheatManager>(GetWorld()->GetFirstPlayerController()->CheatManager) &&
		Cast<UInflectionPointCheatManager>(GetWorld()->GetFirstPlayerController()->CheatManager)->IsCharacterDebugArrowsEnabled) {DebugPrint(__FILE__, __LINE__);
		FRotator cameraRot = FirstPersonCameraComponent->GetComponentRotation();
		FVector cameraDirectionVector = cameraRot.Vector() * 15 + GetTransform().GetLocation();

		//DebugArrowColor.B
		DrawDebugDirectionalArrow(GetWorld(), GetTransform().GetLocation(), cameraDirectionVector, DebugArrowColor.B, DebugArrowColor, true, -1, 0, 0.5f);
	DebugPrint(__FILE__, __LINE__);}
DebugPrint(__FILE__, __LINE__);}

void ABaseCharacter::MulticastOnDeath_Implementation() {DebugPrint(__FILE__, __LINE__);
	// Play random Death Animation
	if(DeathAnimations.Num() == 0) {DebugPrint(__FILE__, __LINE__);
		UE_LOG(LogTemp, Warning, TEXT("Warning: No Death Animations set for this character!"));
	DebugPrint(__FILE__, __LINE__);} else {DebugPrint(__FILE__, __LINE__);
		int index = FMath::RandRange(0, DeathAnimations.Num() - 1);
		Mesh3P->PlayAnimation(DeathAnimations[index], false);
	DebugPrint(__FILE__, __LINE__);}

	// Disable all input
	DisableInput((APlayerController*)GetController());
	DisableComponentsSimulatePhysics();

	GetCapsuleComponent()->SetCollisionProfileName(FName("DeadCharacter"));
DebugPrint(__FILE__, __LINE__);}

void ABaseCharacter::ClientOnDeath_Implementation() {DebugPrint(__FILE__, __LINE__);
	Mesh3P->SetOwnerNoSee(false);
	Mesh1P->SetVisibility(false, true);
	FirstPersonCameraComponent->SetRelativeLocation(FVector(-200.f, 1.75f, 130.f));
	if(!AssertNotNull(GetController(), GetWorld(), __FILE__, __LINE__))
		return;
	FRotator rot = GetController()->GetControlRotation();
	GetController()->SetControlRotation(FRotator(-30, rot.Yaw, rot.Roll));
DebugPrint(__FILE__, __LINE__);}

void ABaseCharacter::MoveForward(float value) {DebugPrint(__FILE__, __LINE__);
	if(value != 0.0f) {DebugPrint(__FILE__, __LINE__);
		AddMovementInput(GetActorForwardVector(), value);
	DebugPrint(__FILE__, __LINE__);}

	if(ShouldStartSprinting(value)) {DebugPrint(__FILE__, __LINE__);
		StartSprinting();
		ServerStartSprinting();
	DebugPrint(__FILE__, __LINE__);} else if(ShouldStopSprinting(value)) {DebugPrint(__FILE__, __LINE__);
		StopSprinting();
		ServerStopSprinting();
	DebugPrint(__FILE__, __LINE__);}
DebugPrint(__FILE__, __LINE__);}

void ABaseCharacter::MoveRight(float value) {DebugPrint(__FILE__, __LINE__);
	if(value != 0.0f) {DebugPrint(__FILE__, __LINE__);
		AddMovementInput(GetActorRightVector(), value);
	DebugPrint(__FILE__, __LINE__);}
DebugPrint(__FILE__, __LINE__);}

void ABaseCharacter::TurnAtRate(float rate) {DebugPrint(__FILE__, __LINE__);
	// calculate delta for this frame from the rate information
	AddControllerYawInput(rate);
DebugPrint(__FILE__, __LINE__);}

void ABaseCharacter::LookUpAtRate(float rate) {DebugPrint(__FILE__, __LINE__);
	// calculate delta for this frame from the rate information
	AddControllerPitchInput(rate);
	ServerUpdateCameraPitch(FirstPersonCameraComponent->GetComponentRotation().Pitch);
	MulticastUpdateCameraPitch(FirstPersonCameraComponent->GetComponentRotation().Pitch);
DebugPrint(__FILE__, __LINE__);}

bool ABaseCharacter::ServerUpdateCameraPitch_Validate(float pitch) {DebugPrint(__FILE__, __LINE__);
	return true;
DebugPrint(__FILE__, __LINE__);}

void ABaseCharacter::ServerUpdateCameraPitch_Implementation(float pitch) {DebugPrint(__FILE__, __LINE__);
	auto currentRot = FirstPersonCameraComponent->GetComponentRotation();
	currentRot.Pitch = pitch;
	currentRot.Roll = 0;
	currentRot.Yaw = 0;
	FirstPersonCameraComponent->SetRelativeRotation(currentRot);
DebugPrint(__FILE__, __LINE__);}

void ABaseCharacter::MulticastUpdateCameraPitch_Implementation(float pitch) {DebugPrint(__FILE__, __LINE__);
	auto currentRot = FirstPersonCameraComponent->GetComponentRotation();
	currentRot.Pitch = pitch;
	currentRot.Roll = 0;
	currentRot.Yaw = 0;
	FirstPersonCameraComponent->SetRelativeRotation(currentRot);
DebugPrint(__FILE__, __LINE__);}

void ABaseCharacter::EnableSprint() {DebugPrint(__FILE__, __LINE__);
	sprintEnabled = true;
DebugPrint(__FILE__, __LINE__);}

void ABaseCharacter::DisableSprint() {DebugPrint(__FILE__, __LINE__);
	sprintEnabled = false;
DebugPrint(__FILE__, __LINE__);}

bool ABaseCharacter::ShouldStartSprinting(float ForwardMovement) {DebugPrint(__FILE__, __LINE__);
	return sprintAllowed && sprintEnabled && ForwardMovement > 0 && GetVelocity().Size() <= walkSpeed;
DebugPrint(__FILE__, __LINE__);}

bool ABaseCharacter::ShouldStopSprinting(float ForwardMovement) {DebugPrint(__FILE__, __LINE__);
	if(GetVelocity().Size() <= walkSpeed) {DebugPrint(__FILE__, __LINE__);
		return false;
	DebugPrint(__FILE__, __LINE__);}
	return !sprintAllowed || !sprintEnabled || ForwardMovement <= 0;
DebugPrint(__FILE__, __LINE__);}

void ABaseCharacter::StartSprinting() {DebugPrint(__FILE__, __LINE__);
	GetCharacterMovement()->MaxWalkSpeed = SprintSpeed;
DebugPrint(__FILE__, __LINE__);}

void ABaseCharacter::StopSprinting() {DebugPrint(__FILE__, __LINE__);
	GetCharacterMovement()->MaxWalkSpeed = walkSpeed;
DebugPrint(__FILE__, __LINE__);}

bool ABaseCharacter::ServerStartSprinting_Validate() {DebugPrint(__FILE__, __LINE__);
	return true;
DebugPrint(__FILE__, __LINE__);}

void ABaseCharacter::ServerStartSprinting_Implementation() {DebugPrint(__FILE__, __LINE__);
	StartSprinting();
DebugPrint(__FILE__, __LINE__);}

bool ABaseCharacter::ServerStopSprinting_Validate() {DebugPrint(__FILE__, __LINE__);
	return true;
DebugPrint(__FILE__, __LINE__);}

void ABaseCharacter::ServerStopSprinting_Implementation() {DebugPrint(__FILE__, __LINE__);
	StopSprinting();
DebugPrint(__FILE__, __LINE__);}

