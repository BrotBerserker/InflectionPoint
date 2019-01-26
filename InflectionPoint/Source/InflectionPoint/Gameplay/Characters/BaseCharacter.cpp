// Copyright 1998-2016 Epic Games, Inc. All Rights Reserved.

#include "InflectionPoint.h"
#include "BaseCharacter.h"
#include "Gameplay/Characters/ReplayCharacterBase.h"
#include "Gameplay/Weapons/InflectionPointProjectile.h"
#include "Gameplay/Weapons/WeaponInventory.h"
#include "Animation/AnimInstance.h"
#include "GameFramework/InputSettings.h"
#include "HeadMountedDisplayFunctionLibrary.h"
#include "MotionControllerComponent.h"
#include "Gameplay/Damage/MortalityProvider.h"
#include "Gameplay/Recording/PlayerStateRecorder.h"
#include "Utils/CheckFunctions.h"
#include "Gamemodes/TDMPlayerStateBase.h"
#include "DebugTools/InflectionPointCheatManager.h"
#include "UI/HUD/CharacterHeadDisplayBase.h"
#include "Kismet/KismetMathLibrary.h"
#include "Gamemodes/TDMGameStateBase.h"
#include "Engine/Classes/Animation/SkeletalMeshActor.h"

DEFINE_LOG_CATEGORY_STATIC(LogFPChar, Warning, All);

void ABaseCharacter::GetLifetimeReplicatedProps(TArray< FLifetimeProperty > & OutLifetimeProps) const {DebugPrint(__FILE__, __LINE__);
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ABaseCharacter, CurrentWeapon);
DebugPrint(__FILE__, __LINE__);}

//////////////////////////////////////////////////////////////////////////
// ABaseCharacter

ABaseCharacter::ABaseCharacter() {DebugPrint(__FILE__, __LINE__);
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(55.f, 89.f);

	// Create a CameraComponent	
	FirstPersonCameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("FirstPersonCamera"));
	FirstPersonCameraComponent->SetupAttachment(GetCapsuleComponent());
	FirstPersonCameraComponent->RelativeLocation = FVector(-16.7f, -1.18f, 49.f); // Position the camera
	FirstPersonCameraComponent->RelativeScale3D = FVector(0.4, 0.4, 0.4); // Scale of the camera
	FirstPersonCameraComponent->bUsePawnControlRotation = true;

	// Create a mesh component that will be used when being viewed from a '1st person' view (when controlling this pawn)
	Mesh1P = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("CharacterMesh1P"));
	Mesh1P->SetOnlyOwnerSee(true);
	Mesh1P->SetupAttachment(FirstPersonCameraComponent);
	Mesh1P->bCastDynamicShadow = false;
	Mesh1P->CastShadow = false;
	Mesh1P->RelativeLocation = FVector(-10.6f, 0.81f, -125.7f);
	Mesh1P->RelativeRotation = FRotator(1.9f, -19.19f, 5.2f);

	// Create the '3rd person' body mesh
	Mesh3P = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("CharacterMesh3P"));
	Mesh3P->SetupAttachment(GetCapsuleComponent());
	Mesh3P->SetOwnerNoSee(true);
	Mesh3P->RelativeLocation = FVector(3.8f, -2.93f, -89.f);
	Mesh3P->RelativeRotation = FRotator(0.f, -90.f, 0.f);

	WeaponInventory = CreateDefaultSubobject<UWeaponInventory>(TEXT("WeaponInventory"));

	// Initialize MortalityProvider
	MortalityProvider = CreateDefaultSubobject<UMortalityProvider>(TEXT("MortalityProvider"));
	MortalityProvider->SetIsReplicated(true);
	MortalityProvider->StartHealth = 100;
	MortalityProvider->StartShield = 50;

	// Initialize Materialize Timeline (wtf aber ok, siehe https://wiki.unrealengine.com/Timeline_in_c%2B%2B)
	MaterializeTimeline = CreateDefaultSubobject<UTimelineComponent>(TEXT("MaterializeTimeline"));

	GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;

	CharacterInfoProvider = CreateDefaultSubobject<UCharacterInfoProvider>(TEXT("CharacterInfoProvider"));
	CharacterInfoProvider->SetIsReplicated(true);

	CharacterHeadDisplay = CreateDefaultSubobject<UWidgetComponent>(TEXT("CharacterHeadDisplay"));
	CharacterHeadDisplay->SetupAttachment(GetCapsuleComponent());
	CharacterHeadDisplay->SetOwnerNoSee(true);
	CharacterHeadDisplay->SetRelativeLocation(FVector(0.f, 0.f, 100.f));
	CharacterHeadDisplay->SetWidgetSpace(EWidgetSpace::Screen);
	CharacterHeadDisplay->SetVisibility(true);
	
	TargetMarkerParticles = CreateDefaultSubobject<UParticleSystemComponent>(TEXT("TargetMarkerParticles"));
	TargetMarkerParticles->SetupAttachment(GetCapsuleComponent());
	TargetMarkerParticles->SetOwnerNoSee(true);
	TargetMarkerParticles->SetVisibility(false);
DebugPrint(__FILE__, __LINE__);}

void ABaseCharacter::BeginPlay() {DebugPrint(__FILE__, __LINE__);
	// Call the base class  
	Super::BeginPlay();

	// Show or hide the two versions of the gun based on whether or not we're using motion controllers.
	Mesh1P->SetHiddenInGame(false, true);
	InitCharacterHeadDisplay();
	
	if(Cast<AReplayCharacterBase>(this) || !IsLocallyControlled()) {DebugPrint(__FILE__, __LINE__);
		FActorSpawnParameters params;
		ASkeletalMeshActor* meshActor = GetWorld()->SpawnActor<ASkeletalMeshActor>(params);
		Cast<USkinnedMeshComponent>(meshActor->GetSkeletalMeshComponent())->SetSkeletalMesh(Mesh3P->SkeletalMesh);
		meshActor->GetSkeletalMeshComponent()->SetMasterPoseComponent(Mesh3P);
		meshActor->AttachToComponent(Mesh3P, FAttachmentTransformRules(EAttachmentRule::SnapToTarget, true));
		meshActor->SetActorEnableCollision(false);
		meshActor->SetActorHiddenInGame(true);
		TargetMarkerParticles->SetActorParameter(FName("VertSurfaceActor"), meshActor);
	DebugPrint(__FILE__, __LINE__);}
DebugPrint(__FILE__, __LINE__);}

bool ABaseCharacter::IsReadyForInitialization() {DebugPrint(__FILE__, __LINE__);
	return WeaponInventory->IsReadyForInitialization();
DebugPrint(__FILE__, __LINE__);}

void ABaseCharacter::Initialize() {DebugPrint(__FILE__, __LINE__);
	if(IsLocallyControlled()) {DebugPrint(__FILE__, __LINE__);
		ServerEquipNextWeapon();
	DebugPrint(__FILE__, __LINE__);}
DebugPrint(__FILE__, __LINE__);}

void ABaseCharacter::InitCharacterHeadDisplay() {DebugPrint(__FILE__, __LINE__);
	if(IsLocallyControlled() && !IsAReplay()) {DebugPrint(__FILE__, __LINE__);
		CharacterHeadDisplay->SetVisibility(false);
		return;
	DebugPrint(__FILE__, __LINE__);}
	CharacterHeadDisplay->AttachToComponent(GetCapsuleComponent(), FAttachmentTransformRules::KeepRelativeTransform); // because unreal ...
	CharacterHeadDisplay->InitWidget();
	auto headDisplayWidget = Cast<UCharacterHeadDisplayBase>(CharacterHeadDisplay->GetUserWidgetObject());
	headDisplayWidget->OwningCharacter = this;
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
	WeaponInventory->ServerDestroy();
	Super::Destroyed();
DebugPrint(__FILE__, __LINE__);}

void ABaseCharacter::UpdateFieldOfView(float DeltaTime) {DebugPrint(__FILE__, __LINE__);
	float targetFoV = IsAiming && CurrentWeapon ? CurrentWeapon->AimFieldOfView : 90.f;
	FirstPersonCameraComponent->SetFieldOfView(FMath::FInterpTo(FirstPersonCameraComponent->FieldOfView, targetFoV, DeltaTime, 14.f));
DebugPrint(__FILE__, __LINE__);}

bool ABaseCharacter::IsAReplay() {DebugPrint(__FILE__, __LINE__);
	return this->IsA(AReplayCharacterBase::StaticClass());
DebugPrint(__FILE__, __LINE__);}

bool ABaseCharacter::IsInSameTeamAs(class ABaseCharacter* character) {DebugPrint(__FILE__, __LINE__);
	return CharacterInfoProvider->IsInSameTeamAs(character);
DebugPrint(__FILE__, __LINE__);}

void ABaseCharacter::ShowSpawnAnimation() {DebugPrint(__FILE__, __LINE__);
	if(!SoftAssertTrue(MaterializeCurve != nullptr, GetWorld(), __FILE__, __LINE__, TEXT("No materialize curve has been set!")))
		return;

	// add curve to timeline
	FOnTimelineFloat callback{};
	callback.BindUFunction(this, FName{TEXT("MaterializeCallback")});
	MaterializeTimeline->AddInterpFloat(MaterializeCurve, callback, FName{TEXT("MaterializeTimelineAnimation") });

	// set timeline finish callback
	FOnTimelineEvent finishCallback{};
	finishCallback.BindUFunction(this, FName{TEXT("MaterializeFinishCallback") });
	MaterializeTimeline->SetTimelineFinishedFunc(finishCallback);

	MaterializeTimeline->Play();
DebugPrint(__FILE__, __LINE__);}

void ABaseCharacter::MaterializeCallback(float value) {DebugPrint(__FILE__, __LINE__);
	DynamicBodyMaterial->SetScalarParameterValue("Materialize Amount", value);
DebugPrint(__FILE__, __LINE__);}

void ABaseCharacter::MaterializeFinishCallback() {DebugPrint(__FILE__, __LINE__);
	// Switch to materials without materialize effect to save a lot of performance
	if(!BodyMaterialAfterMaterialize) {DebugPrint(__FILE__, __LINE__);
		return;
	DebugPrint(__FILE__, __LINE__);}
	UMaterialInstanceDynamic* dynamicMaterialWithoutMaterialize = UMaterialInstanceDynamic::Create(BodyMaterialAfterMaterialize, Mesh3P);
	FLinearColor bodyColor;
	DynamicBodyMaterial->GetVectorParameterValue(FMaterialParameterInfo("BodyColor"), bodyColor);
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
	if(EventInstigator && EventInstigator->GetCharacter()) {DebugPrint(__FILE__, __LINE__);
		FVector directionVector = (EventInstigator->GetCharacter()->GetActorLocation() - GetActorLocation());
		directionVector.Normalize();
		OnDirectionalDamageReceived(directionVector, actualDamage, DamageEvent.DamageTypeClass);
	DebugPrint(__FILE__, __LINE__);}
	return actualDamage;
DebugPrint(__FILE__, __LINE__);}

void ABaseCharacter::StartFire() {DebugPrint(__FILE__, __LINE__);
	DisableSprint();
	sprintAllowed = false;
	ServerStartFire();
DebugPrint(__FILE__, __LINE__);}

bool ABaseCharacter::ServerFireOnce_Validate() {DebugPrint(__FILE__, __LINE__);
	return true;
DebugPrint(__FILE__, __LINE__);}

void ABaseCharacter::ServerFireOnce_Implementation() {DebugPrint(__FILE__, __LINE__);
	DrawDebugArrow();
	if(!AssertNotNull(CurrentWeapon, GetWorld(), __FILE__, __LINE__))
		return;
	CurrentWeapon->FireOnce();
DebugPrint(__FILE__, __LINE__);}

bool ABaseCharacter::ServerStartFire_Validate() {DebugPrint(__FILE__, __LINE__);
	return true;
DebugPrint(__FILE__, __LINE__);}

void ABaseCharacter::ServerStartFire_Implementation() {DebugPrint(__FILE__, __LINE__);
	DrawDebugArrow();
	if(!CurrentWeapon)
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
	if(CurrentWeapon)
		CurrentWeapon->StopFire();
DebugPrint(__FILE__, __LINE__);}

void ABaseCharacter::StartAiming() {DebugPrint(__FILE__, __LINE__);
	IsAiming = true;
	if(CurrentWeapon)
		CurrentWeapon->StartAiming();
	auto controller = Cast<APlayerControllerBase>(GetController());
	if(controller)
		controller->OnStartAiming(CurrentWeapon);
	ServerStartAiming();
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
	if(CurrentWeapon)
		CurrentWeapon->StartAiming();
	auto controller = Cast<APlayerControllerBase>(GetWorld()->GetFirstPlayerController());
	if(controller && controller->SpectatedCharacter == this) {DebugPrint(__FILE__, __LINE__);
		controller->OnStartAiming(CurrentWeapon);
	DebugPrint(__FILE__, __LINE__);}
DebugPrint(__FILE__, __LINE__);}

void ABaseCharacter::StopAiming() {DebugPrint(__FILE__, __LINE__);
	IsAiming = false;
	if(CurrentWeapon)
		CurrentWeapon->StopAiming();
	auto controller = Cast<APlayerControllerBase>(GetController());
	if(controller)
		controller->OnStopAiming(CurrentWeapon);
	ServerStopAiming();
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
	if(CurrentWeapon)
		CurrentWeapon->StopAiming();
	auto controller = Cast<APlayerControllerBase>(GetWorld()->GetFirstPlayerController());
	if(controller && controller->SpectatedCharacter == this)
		controller->OnStopAiming(CurrentWeapon);
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
	EquipWeapon(WeaponInventory->GetNextUsableWeapon(CurrentWeapon), CurrentWeapon);
DebugPrint(__FILE__, __LINE__);}

bool ABaseCharacter::ServerEquipPreviousWeapon_Validate() {DebugPrint(__FILE__, __LINE__);
	return true;
DebugPrint(__FILE__, __LINE__);}

void ABaseCharacter::ServerEquipPreviousWeapon_Implementation() {DebugPrint(__FILE__, __LINE__);
	EquipWeapon(WeaponInventory->GetPreviousUsableWeapon(CurrentWeapon), CurrentWeapon);
DebugPrint(__FILE__, __LINE__);}

bool ABaseCharacter::ServerEquipSpecificWeapon_Validate(EInventorySlotPosition slot) {DebugPrint(__FILE__, __LINE__);
	return true;
DebugPrint(__FILE__, __LINE__);}

void ABaseCharacter::ServerEquipSpecificWeapon_Implementation(EInventorySlotPosition slot) {DebugPrint(__FILE__, __LINE__);
	ABaseWeapon* newWeapon = WeaponInventory->GetWeapon(slot);
	if(newWeapon && CurrentWeapon != newWeapon)
		EquipWeapon(newWeapon, CurrentWeapon);
DebugPrint(__FILE__, __LINE__);}

bool ABaseCharacter::ServerEquipRandomWeapon_Validate() {DebugPrint(__FILE__, __LINE__);
	return true;
DebugPrint(__FILE__, __LINE__);}

void ABaseCharacter::ServerEquipRandomWeapon_Implementation() {DebugPrint(__FILE__, __LINE__);
	ABaseWeapon* newWeapon = WeaponInventory->GetRandomWeapon();
	if(newWeapon && CurrentWeapon != newWeapon)
		EquipWeapon(newWeapon, CurrentWeapon);
DebugPrint(__FILE__, __LINE__);}

bool ABaseCharacter::ServerPickWeaponUp_Validate(UClass* weapon) {DebugPrint(__FILE__, __LINE__);
	return true;
DebugPrint(__FILE__, __LINE__);}

void ABaseCharacter::ServerPickWeaponUp_Implementation(UClass* weapon) {DebugPrint(__FILE__, __LINE__);
	auto newWeapon = WeaponInventory->GetWeaponByClass(weapon);
	if(newWeapon)
		EquipWeapon(newWeapon, CurrentWeapon);
DebugPrint(__FILE__, __LINE__);}

void ABaseCharacter::OnRep_CurrentWeapon(ABaseWeapon* OldWeapon) {DebugPrint(__FILE__, __LINE__);
	EquipWeapon(CurrentWeapon, OldWeapon);
DebugPrint(__FILE__, __LINE__);}

void ABaseCharacter::EquipWeapon(ABaseWeapon* NewWeapon, ABaseWeapon* OldWeapon) {DebugPrint(__FILE__, __LINE__);
	CurrentWeapon = NewWeapon;

	if(OldWeapon) {DebugPrint(__FILE__, __LINE__);
		if(IsAiming) {DebugPrint(__FILE__, __LINE__);
			OldWeapon->StopAiming();
		DebugPrint(__FILE__, __LINE__);}
		Mesh1P->GetAnimInstance()->OnPlayMontageNotifyBegin.Remove(CurrentWeapon->AnimationNotifyDelegate);
		Mesh1P->GetAnimInstance()->Montage_Stop(0, CurrentWeapon->ReloadAnimation1P);
		OldWeapon->OnUnequip();
	DebugPrint(__FILE__, __LINE__);}
	
	Mesh1P->GetAnimInstance()->Montage_Play(CurrentWeapon->EquipAnimation1P);
	Mesh3P->GetAnimInstance()->Montage_Play(CurrentWeapon->EquipAnimation3P);

	if(IsAiming) {DebugPrint(__FILE__, __LINE__);
		CurrentWeapon->StartAiming();
	DebugPrint(__FILE__, __LINE__);}
	CurrentWeapon->OnEquip();

	MulticastWeaponChanged(NewWeapon, OldWeapon);
DebugPrint(__FILE__, __LINE__);}

void ABaseCharacter::MulticastWeaponChanged_Implementation(ABaseWeapon* newWeapon, ABaseWeapon* oldWeapon) {DebugPrint(__FILE__, __LINE__);
	auto controller = Cast<APlayerControllerBase>(GetController());
	if(controller)
		controller->OnWeaponChanged(newWeapon, oldWeapon);
	controller = Cast<APlayerControllerBase>(GetWorld()->GetFirstPlayerController());
	if(controller && controller->SpectatedCharacter == this)
		controller->OnWeaponChanged(newWeapon, oldWeapon);
DebugPrint(__FILE__, __LINE__);}

void ABaseCharacter::DrawDebugArrow() {DebugPrint(__FILE__, __LINE__);
	if(Cast<UInflectionPointCheatManager>(GetWorld()->GetFirstPlayerController()->CheatManager) &&
		Cast<UInflectionPointCheatManager>(GetWorld()->GetFirstPlayerController()->CheatManager)->IsCharacterDebugArrowsEnabled) {DebugPrint(__FILE__, __LINE__);
		FRotator cameraRot = FirstPersonCameraComponent->GetComponentRotation();
		FVector cameraDirectionVector = cameraRot.Vector() * 15 + GetTransform().GetLocation();

		DrawDebugDirectionalArrow(GetWorld(), GetTransform().GetLocation(), cameraDirectionVector, DebugArrowColor.B, DebugArrowColor, true, -1, 0, 0.5f);
	DebugPrint(__FILE__, __LINE__);}
DebugPrint(__FILE__, __LINE__);}

void ABaseCharacter::MulticastOnDeath_Implementation(bool suicide) {DebugPrint(__FILE__, __LINE__);
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
	PlayDeathSound(suicide);
DebugPrint(__FILE__, __LINE__);}

void ABaseCharacter::MulticastPlay3PAnimation_Implementation(UAnimationAsset* animation) {DebugPrint(__FILE__, __LINE__);
	Mesh3P->PlayAnimation(animation, false);
DebugPrint(__FILE__, __LINE__);}

void ABaseCharacter::ClientOnDeath_Implementation() {DebugPrint(__FILE__, __LINE__);
	Mesh3P->SetOwnerNoSee(false);
	Mesh1P->SetVisibility(false, true);
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
	return sprintAllowed && sprintEnabled && ForwardMovement > 0 && GetVelocity().Size() <= WalkSpeed;
DebugPrint(__FILE__, __LINE__);}

bool ABaseCharacter::ShouldStopSprinting(float ForwardMovement) {DebugPrint(__FILE__, __LINE__);
	if(GetVelocity().Size() <= WalkSpeed) {DebugPrint(__FILE__, __LINE__);
		return false;
	DebugPrint(__FILE__, __LINE__);}
	return !sprintAllowed || !sprintEnabled || ForwardMovement <= 0;
DebugPrint(__FILE__, __LINE__);}

void ABaseCharacter::StartSprinting() {DebugPrint(__FILE__, __LINE__);
	GetCharacterMovement()->MaxWalkSpeed = SprintSpeed;
DebugPrint(__FILE__, __LINE__);}

void ABaseCharacter::StopSprinting() {DebugPrint(__FILE__, __LINE__);
	GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;
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

bool ABaseCharacter::IsAlive() {DebugPrint(__FILE__, __LINE__);
	return MortalityProvider && MortalityProvider->IsAlive();
DebugPrint(__FILE__, __LINE__);}

bool ABaseCharacter::IsInSameTeamAsLocalPlayer() {DebugPrint(__FILE__, __LINE__);
	auto localPlayerState = Cast<ATDMPlayerStateBase>(GetWorld()->GetFirstPlayerController()->PlayerState);
	if(!localPlayerState)
		return false;
	return localPlayerState->Team == CharacterInfoProvider->GetCharacterInfo().Team;
DebugPrint(__FILE__, __LINE__);}

UCharacterHeadDisplayBase* ABaseCharacter::GetCharacterHeadDisplay() {DebugPrint(__FILE__, __LINE__);
	return Cast<UCharacterHeadDisplayBase>(CharacterHeadDisplay->GetUserWidgetObject());
DebugPrint(__FILE__, __LINE__);}


void ABaseCharacter::PlayDeathSound(bool suicide) {DebugPrint(__FILE__, __LINE__);
	auto audio = suicide ? SuicideSound : DeathSound;
	if(audio)
		UGameplayStatics::SpawnSoundAttached(audio, Mesh3P);
DebugPrint(__FILE__, __LINE__);}