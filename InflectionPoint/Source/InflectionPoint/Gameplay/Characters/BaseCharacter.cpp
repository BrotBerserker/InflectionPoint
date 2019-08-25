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

void ABaseCharacter::GetLifetimeReplicatedProps(TArray< FLifetimeProperty > & OutLifetimeProps) const {
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ABaseCharacter, CurrentWeapon);
}

//////////////////////////////////////////////////////////////////////////
// ABaseCharacter

ABaseCharacter::ABaseCharacter() {
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

	CrouchingTimeline = CreateDefaultSubobject<UTimelineComponent>(TEXT("CrouchingTimeline"));

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
}

void ABaseCharacter::BeginPlay() {
	// Call the base class  
	Super::BeginPlay();

	// Show or hide the two versions of the gun based on whether or not we're using motion controllers.
	Mesh1P->SetHiddenInGame(false, true);
	InitCharacterHeadDisplay();

	if(Cast<AReplayCharacterBase>(this) || !IsLocallyControlled()) {
		FActorSpawnParameters params;
		ASkeletalMeshActor* meshActor = GetWorld()->SpawnActor<ASkeletalMeshActor>(params);
		Cast<USkinnedMeshComponent>(meshActor->GetSkeletalMeshComponent())->SetSkeletalMesh(Mesh3P->SkeletalMesh);
		meshActor->GetSkeletalMeshComponent()->SetMasterPoseComponent(Mesh3P);
		meshActor->AttachToComponent(Mesh3P, FAttachmentTransformRules(EAttachmentRule::SnapToTarget, true));
		meshActor->SetActorEnableCollision(false);
		meshActor->SetActorHiddenInGame(true);
		TargetMarkerParticles->SetActorParameter(FName("VertSurfaceActor"), meshActor);
	}

	AssertNotNull(CrouchingCurve, GetWorld(), __FILE__, __LINE__);

	FOnTimelineFloat callback{};
	callback.BindUFunction(this, FName{ TEXT("CrouchingCallback") });
	CrouchingTimeline->AddInterpFloat(CrouchingCurve, callback, FName{ TEXT("CrouchingTimelineAnimation") });

	CamMiddle = FirstPersonCameraComponent->RelativeLocation;
	CrouchHeightDiff = GetCapsuleComponent()->GetScaledCapsuleHalfHeight() - GetCharacterMovement()->CrouchedHalfHeight;
	CamTop = FVector(CamMiddle.X, CamMiddle.Y, CamMiddle.Z + CrouchHeightDiff);
	CamBottom = FVector(CamMiddle.X, CamMiddle.Y, CamMiddle.Z - CrouchHeightDiff);
}

bool ABaseCharacter::IsReadyForInitialization() {
	return WeaponInventory->IsReadyForInitialization();
}

void ABaseCharacter::Initialize() {
	if(IsLocallyControlled()) {
		ServerEquipNextWeapon();
	}
}

void ABaseCharacter::InitCharacterHeadDisplay() {
	if(IsLocallyControlled() && !IsAReplay()) {
		CharacterHeadDisplay->SetVisibility(false);
		return;
	}
	CharacterHeadDisplay->AttachToComponent(GetCapsuleComponent(), FAttachmentTransformRules::KeepRelativeTransform); // because unreal ...
	CharacterHeadDisplay->InitWidget();
	auto headDisplayWidget = Cast<UCharacterHeadDisplayBase>(CharacterHeadDisplay->GetUserWidgetObject());
	headDisplayWidget->OwningCharacter = this;
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

	if(wantsToCrouch && GetCharacterMovement()->IsMovingOnGround()) {
		wantsToCrouch = false;
		Crouch();
	}
}

void ABaseCharacter::Destroyed() {
	WeaponInventory->ServerDestroy();
	Super::Destroyed();
}

void ABaseCharacter::UpdateFieldOfView(float DeltaTime) {
	float targetFoV = IsAiming && CurrentWeapon ? CurrentWeapon->AimFieldOfView : 90.f;
	FirstPersonCameraComponent->SetFieldOfView(FMath::FInterpTo(FirstPersonCameraComponent->FieldOfView, targetFoV, DeltaTime, 14.f));
}

bool ABaseCharacter::IsAReplay() {
	return this->IsA(AReplayCharacterBase::StaticClass());
}

bool ABaseCharacter::IsInSameTeamAs(class ABaseCharacter* character) {
	return CharacterInfoProvider->IsInSameTeamAs(character);
}

void ABaseCharacter::ShowSpawnAnimation() {
	if(!SoftAssertTrue(MaterializeCurve != nullptr, GetWorld(), __FILE__, __LINE__, TEXT("No materialize curve has been set!")))
		return;

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
	if(!BodyMaterialAfterMaterialize) {
		return;
	}
	UMaterialInstanceDynamic* dynamicMaterialWithoutMaterialize = UMaterialInstanceDynamic::Create(BodyMaterialAfterMaterialize, Mesh3P);
	FLinearColor bodyColor;
	DynamicBodyMaterial->GetVectorParameterValue(FMaterialParameterInfo("BodyColor"), bodyColor);
	dynamicMaterialWithoutMaterialize->SetVectorParameterValue("BodyColor", bodyColor);
	Mesh3P->SetMaterial(0, dynamicMaterialWithoutMaterialize);
	Mesh1P->SetMaterial(0, dynamicMaterialWithoutMaterialize);
}

void ABaseCharacter::MulticastShowSpawnAnimation_Implementation() {
	ShowSpawnAnimation();
}

float ABaseCharacter::TakeDamage(float DamageAmount, FDamageEvent const & DamageEvent, AController * EventInstigator, AActor * DamageCauser) {
	const float actualDamage = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);
	if(EventInstigator && EventInstigator->GetCharacter()) {
		FVector directionVector = (EventInstigator->GetCharacter()->GetActorLocation() - GetActorLocation());
		directionVector.Normalize();
		OnDirectionalDamageReceived(directionVector, actualDamage, DamageEvent.DamageTypeClass);
	}
	return actualDamage;
}

void ABaseCharacter::StartFire(EFireMode mode) {
	DisableSprint();
	sprintAllowed = false;
	ServerStartFire(mode);
}

bool ABaseCharacter::ServerFireOnce_Validate(EFireMode mode) {
	return true;
}

void ABaseCharacter::ServerFireOnce_Implementation(EFireMode mode) {
	DrawDebugArrow();
	if(!AssertNotNull(CurrentWeapon, GetWorld(), __FILE__, __LINE__))
		return;
	CurrentWeapon->FireOnce(mode);
}

bool ABaseCharacter::ServerStartFire_Validate(EFireMode mode) {
	return true;
}

void ABaseCharacter::ServerStartFire_Implementation(EFireMode mode) {
	DrawDebugArrow();
	if(!CurrentWeapon)
		return;
	CurrentWeapon->StartFire(mode);
}

void ABaseCharacter::StopFire(EFireMode mode) {
	sprintAllowed = true;
	ServerStopFire(mode);
}

bool ABaseCharacter::ServerStopFire_Validate(EFireMode mode) {
	return true;
}

void ABaseCharacter::ServerStopFire_Implementation(EFireMode mode) {
	if(CurrentWeapon)
		CurrentWeapon->StopFire(mode);
}

void ABaseCharacter::StartAiming() {
	IsAiming = true;
	if(CurrentWeapon)
		CurrentWeapon->StartAiming();
	auto controller = Cast<APlayerControllerBase>(GetController());
	if(controller)
		controller->OnStartAiming(CurrentWeapon);
	ServerStartAiming();
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
	if(CurrentWeapon)
		CurrentWeapon->StartAiming();
	auto controller = Cast<APlayerControllerBase>(GetWorld()->GetFirstPlayerController());
	if(controller && controller->SpectatedCharacter == this) {
		controller->OnStartAiming(CurrentWeapon);
	}
}

void ABaseCharacter::StopAiming() {
	IsAiming = false;
	if(CurrentWeapon)
		CurrentWeapon->StopAiming();
	auto controller = Cast<APlayerControllerBase>(GetController());
	if(controller)
		controller->OnStopAiming(CurrentWeapon);
	ServerStopAiming();
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
	if(CurrentWeapon)
		CurrentWeapon->StopAiming();
	auto controller = Cast<APlayerControllerBase>(GetWorld()->GetFirstPlayerController());
	if(controller && controller->SpectatedCharacter == this)
		controller->OnStopAiming(CurrentWeapon);
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
	EquipWeapon(WeaponInventory->GetNextUsableWeapon(CurrentWeapon), CurrentWeapon);
}

bool ABaseCharacter::ServerEquipPreviousWeapon_Validate() {
	return true;
}

void ABaseCharacter::ServerEquipPreviousWeapon_Implementation() {
	EquipWeapon(WeaponInventory->GetPreviousUsableWeapon(CurrentWeapon), CurrentWeapon);
}

bool ABaseCharacter::ServerEquipSpecificWeapon_Validate(EInventorySlotPosition slot) {
	return true;
}

void ABaseCharacter::ServerEquipSpecificWeapon_Implementation(EInventorySlotPosition slot) {
	ABaseWeapon* newWeapon = WeaponInventory->GetWeapon(slot);
	if(newWeapon && CurrentWeapon != newWeapon)
		EquipWeapon(newWeapon, CurrentWeapon);
}

bool ABaseCharacter::ServerEquipRandomWeapon_Validate() {
	return true;
}

void ABaseCharacter::ServerEquipRandomWeapon_Implementation() {
	ABaseWeapon* newWeapon = WeaponInventory->GetRandomWeapon();
	if(newWeapon && CurrentWeapon != newWeapon)
		EquipWeapon(newWeapon, CurrentWeapon);
}

bool ABaseCharacter::ServerPickWeaponUp_Validate(UClass* weapon) {
	return true;
}

void ABaseCharacter::ServerPickWeaponUp_Implementation(UClass* weapon) {
	auto newWeapon = WeaponInventory->GetWeaponByClass(weapon);
	if(newWeapon)
		EquipWeapon(newWeapon, CurrentWeapon);
}

void ABaseCharacter::OnRep_CurrentWeapon(ABaseWeapon* OldWeapon) {
	EquipWeapon(CurrentWeapon, OldWeapon);
}

void ABaseCharacter::EquipWeapon(ABaseWeapon* NewWeapon, ABaseWeapon* OldWeapon) {
	CurrentWeapon = NewWeapon;

	if(OldWeapon) {
		if(IsAiming) {
			OldWeapon->StopAiming();
		}
		Mesh1P->GetAnimInstance()->OnPlayMontageNotifyBegin.Remove(CurrentWeapon->AnimationNotifyDelegate);
		Mesh1P->GetAnimInstance()->Montage_Stop(0, CurrentWeapon->ReloadAnimation1P);
		OldWeapon->OnUnequip();
	}

	Mesh1P->GetAnimInstance()->Montage_Play(CurrentWeapon->EquipAnimation1P);
	Mesh3P->GetAnimInstance()->Montage_Play(CurrentWeapon->EquipAnimation3P);

	if(IsAiming) {
		CurrentWeapon->StartAiming();
	}
	CurrentWeapon->OnEquip();

	MulticastWeaponChanged(NewWeapon, OldWeapon);
}

void ABaseCharacter::MulticastWeaponChanged_Implementation(ABaseWeapon* newWeapon, ABaseWeapon* oldWeapon) {
	auto controller = Cast<APlayerControllerBase>(GetController());
	if(controller)
		controller->OnWeaponChanged(newWeapon, oldWeapon);
	controller = Cast<APlayerControllerBase>(GetWorld()->GetFirstPlayerController());
	if(controller && controller->SpectatedCharacter == this)
		controller->OnWeaponChanged(newWeapon, oldWeapon);
}

void ABaseCharacter::DrawDebugArrow() {
	if(Cast<UInflectionPointCheatManager>(GetWorld()->GetFirstPlayerController()->CheatManager) &&
		Cast<UInflectionPointCheatManager>(GetWorld()->GetFirstPlayerController()->CheatManager)->IsCharacterDebugArrowsEnabled) {
		FRotator cameraRot = FirstPersonCameraComponent->GetComponentRotation();
		FVector cameraDirectionVector = cameraRot.Vector() * 15 + GetTransform().GetLocation();

		DrawDebugDirectionalArrow(GetWorld(), GetTransform().GetLocation(), cameraDirectionVector, DebugArrowColor.B, DebugArrowColor, true, -1, 0, 0.5f);
	}
}

void ABaseCharacter::MulticastOnDeath_Implementation(bool suicide) {
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
	PlayDeathSound(suicide);
}

void ABaseCharacter::MulticastPlay3PAnimation_Implementation(UAnimationAsset* animation) {
	Mesh3P->PlayAnimation(animation, false);
}

void ABaseCharacter::ClientOnDeath_Implementation() {
	Mesh3P->SetOwnerNoSee(false);
	Mesh1P->SetVisibility(false, true);
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
	if(bIsCrouched) {
		UnCrouch();
	}
}

void ABaseCharacter::DisableSprint() {
	sprintEnabled = false;
}

bool ABaseCharacter::ShouldStartSprinting(float ForwardMovement) {
	return sprintAllowed && sprintEnabled && ForwardMovement > 0 && GetVelocity().Size() <= WalkSpeed;
}

bool ABaseCharacter::ShouldStopSprinting(float ForwardMovement) {
	if(GetVelocity().Size() <= WalkSpeed) {
		return false;
	}
	return !sprintAllowed || !sprintEnabled || ForwardMovement <= 0;
}

void ABaseCharacter::StartSprinting() {
	GetCharacterMovement()->MaxWalkSpeed = SprintSpeed;
}

void ABaseCharacter::StopSprinting() {
	GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;
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

void ABaseCharacter::Jump() {
	if(bIsCrouched) {
		UnCrouch();
	} else {
		Super::Jump();
	}
}

void ABaseCharacter::ToggleCrouching() {
	if(!bIsCrouched) {
		wantsToCrouch = true;
	} else {
		UnCrouch();
	}
}

void ABaseCharacter::OnStartCrouch(float HalfHeightAdjust, float ScaledHalfHeightAdjust) {
	FVector loc = Mesh3P->RelativeLocation;
	Mesh3P->SetRelativeLocation(FVector(loc.X, loc.Y, loc.Z + CrouchHeightDiff - 7));
	CamStart = CamTop;
	CamFinish = CamMiddle;
	CrouchingTimeline->Play();
}

void ABaseCharacter::OnEndCrouch(float HalfHeightAdjust, float ScaledHalfHeightAdjust) {
	FVector loc = Mesh3P->RelativeLocation;
	Mesh3P->SetRelativeLocation(FVector(loc.X, loc.Y, loc.Z - CrouchHeightDiff + 7));
	CamStart = CamMiddle;
	CamFinish = CamBottom;
	CrouchingTimeline->Reverse();
}

void ABaseCharacter::CrouchingCallback(float value) {
	FirstPersonCameraComponent->SetRelativeLocation(FMath::Lerp(CamFinish, CamStart, value));
}

bool ABaseCharacter::IsAlive() {
	return MortalityProvider && MortalityProvider->IsAlive();
}

bool ABaseCharacter::IsInSameTeamAsLocalPlayer() {
	auto localPlayerState = Cast<ATDMPlayerStateBase>(GetWorld()->GetFirstPlayerController()->PlayerState);
	if(!localPlayerState)
		return false;
	return localPlayerState->Team == CharacterInfoProvider->GetCharacterInfo().Team;
}

UCharacterHeadDisplayBase* ABaseCharacter::GetCharacterHeadDisplay() {
	return Cast<UCharacterHeadDisplayBase>(CharacterHeadDisplay->GetUserWidgetObject());
}


void ABaseCharacter::PlayDeathSound(bool suicide) {
	auto audio = suicide ? SuicideSound : DeathSound;
	if(audio)
		UGameplayStatics::SpawnSoundAttached(audio, Mesh3P);
}

bool ABaseCharacter::IsHitAHeadshot(const FHitResult hitResult) {
	if(this->GetActorLocation().Z > hitResult.ImpactPoint.Z)
		return false;
	return (this->GetActorLocation() - hitResult.ImpactPoint).Size() > 60;
}