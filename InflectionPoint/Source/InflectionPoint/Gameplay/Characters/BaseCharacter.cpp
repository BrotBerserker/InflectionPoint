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
#include "Gamemodes/TDMGameStateBase.h"

DEFINE_LOG_CATEGORY_STATIC(LogFPChar, Warning, All);

void ABaseCharacter::GetLifetimeReplicatedProps(TArray< FLifetimeProperty > & OutLifetimeProps) const {DebugPrint(__FILE__, __LINE__);
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ABaseCharacter, CurrentWeapon);
}

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

	CharacterNameTag = CreateDefaultSubobject<UTextRenderComponent>(TEXT("CharacterNameTag"));
	CharacterNameTag->SetCastShadow(false);
	CharacterNameTag->SetupAttachment(GetCapsuleComponent());
	CharacterNameTag->SetOwnerNoSee(true);
	CharacterNameTag->bCastDynamicShadow = false;
	CharacterNameTag->bAffectDynamicIndirectLighting = false;
	CharacterNameTag->SetRelativeLocation(FVector(0, 0, 90));
	CharacterNameTag->SetHorizontalAlignment(EHorizTextAligment::EHTA_Center);
}

void ABaseCharacter::BeginPlay() {DebugPrint(__FILE__, __LINE__);
	// Call the base class  
	Super::BeginPlay();

	// Show or hide the two versions of the gun based on whether or not we're using motion controllers.
	Mesh1P->SetHiddenInGame(false, true);

	//// Create dynamic materials for materialize animation
	//DynamicBodyMaterial = UMaterialInstanceDynamic::Create(Mesh3P->GetMaterial(0), Mesh3P);
	//Mesh3P->SetMaterial(0, DynamicBodyMaterial);
	//Mesh1P->SetMaterial(0, DynamicBodyMaterial);
}

bool ABaseCharacter::IsReadyForInitialization() {DebugPrint(__FILE__, __LINE__);
	return true;
}

void ABaseCharacter::Initialize() {DebugPrint(__FILE__, __LINE__);
	if(IsLocallyControlled()) {DebugPrint(__FILE__, __LINE__);
		ServerEquipSpecificWeapon(0);
	}
}

void ABaseCharacter::Restart() {DebugPrint(__FILE__, __LINE__);
	Super::Restart();
	OnRestart();
}

void ABaseCharacter::Tick(float DeltaTime) {DebugPrint(__FILE__, __LINE__);
	Super::Tick(DeltaTime);

	if(!initialized && IsReadyForInitialization()) {DebugPrint(__FILE__, __LINE__);
		Initialize();
		OnInitialized();
		initialized = true;
	}

	UpdateFieldOfView(DeltaTime);
	UpdateCharacterNameTag();
}

void ABaseCharacter::Destroyed() {DebugPrint(__FILE__, __LINE__);
	Super::Destroyed();
	WeaponInventory->Destroy();
}

void ABaseCharacter::UpdateFieldOfView(float DeltaTime) {DebugPrint(__FILE__, __LINE__);
	float targetFoV = IsAiming && CurrentWeapon ? CurrentWeapon->AimFieldOfView : 90.f;
	FirstPersonCameraComponent->SetFieldOfView(FMath::FInterpTo(FirstPersonCameraComponent->FieldOfView, targetFoV, DeltaTime, 14.f));
}

bool ABaseCharacter::IsAReplay() {DebugPrint(__FILE__, __LINE__);
	return this->IsA(AReplayCharacterBase::StaticClass());
}

void ABaseCharacter::ApplyTeamColor(ATDMPlayerStateBase* playerState) {DebugPrint(__FILE__, __LINE__);
	ATDMGameStateBase* gameState = Cast<ATDMGameStateBase>(UGameplayStatics::GetGameState(GetWorld()));
	AssertNotNull(gameState, GetWorld(), __FILE__, __LINE__, TEXT("GameState is null!"));

	auto bodyMetalColor = IsAReplay() ? gameState->ReplayTeamColors[playerState->Team] : gameState->TeamColors[playerState->Team];
	ApplyColorToMaterials(Mesh3P, bodyMetalColor);
	ApplyColorToMaterials(Mesh1P, bodyMetalColor);
}


void ABaseCharacter::ApplyColorToMaterials(UMeshComponent* mesh, FLinearColor color) {DebugPrint(__FILE__, __LINE__);
	auto unusedColor = FLinearColor(); // only needed for checking the vecorParameter
	for(int i = 0; i < mesh->GetMaterials().Num(); i++) {DebugPrint(__FILE__, __LINE__);
		bool check = mesh->GetMaterial(i)->GetVectorParameterValue(TeamColorMaterialParameterName, unusedColor);
		if(!check)
			continue;
		UMaterialInstanceDynamic* dynamic = UMaterialInstanceDynamic::Create(mesh->GetMaterial(i), mesh);
		dynamic->SetVectorParameterValue(TeamColorMaterialParameterName, color);
		mesh->SetMaterial(i, dynamic);
	}
}

void ABaseCharacter::MulticastApplyTeamColor_Implementation(ATDMPlayerStateBase* playerState) {DebugPrint(__FILE__, __LINE__);
	ApplyTeamColor(playerState);
}

void ABaseCharacter::ShowSpawnAnimation() {DebugPrint(__FILE__, __LINE__);
	if(!SoftAssertTrue(MaterializeCurve != nullptr, GetWorld(), __FILE__, __LINE__, TEXT("No materialize curve has been set!")))
		return;

	// add curve to timeline
	FOnTimelineFloat callback{};
	callback.BindUFunction(this, FName{TEXT("MaterializeCallback") });
	MaterializeTimeline->AddInterpFloat(MaterializeCurve, callback, FName{TEXT("MaterializeTimelineAnimation") });

	// set timeline finish callback
	FOnTimelineEvent finishCallback{};
	finishCallback.BindUFunction(this, FName{TEXT("MaterializeFinishCallback") });
	MaterializeTimeline->SetTimelineFinishedFunc(finishCallback);

	MaterializeTimeline->Play();
}

void ABaseCharacter::MaterializeCallback(float value) {DebugPrint(__FILE__, __LINE__);
	DynamicBodyMaterial->SetScalarParameterValue("Materialize Amount", value);
}

void ABaseCharacter::MaterializeFinishCallback() {DebugPrint(__FILE__, __LINE__);
	// Switch to materials without materialize effect to save a lot of performance
	if(!BodyMaterialAfterMaterialize) {DebugPrint(__FILE__, __LINE__);
		return;
	}
	UMaterialInstanceDynamic* dynamicMaterialWithoutMaterialize = UMaterialInstanceDynamic::Create(BodyMaterialAfterMaterialize, Mesh3P);
	FLinearColor bodyColor;
	DynamicBodyMaterial->GetVectorParameterValue(FMaterialParameterInfo("BodyColor"), bodyColor);
	dynamicMaterialWithoutMaterialize->SetVectorParameterValue("BodyColor", bodyColor);
	Mesh3P->SetMaterial(0, dynamicMaterialWithoutMaterialize);
	Mesh1P->SetMaterial(0, dynamicMaterialWithoutMaterialize);
}

void ABaseCharacter::MulticastShowSpawnAnimation_Implementation() {DebugPrint(__FILE__, __LINE__);
	ShowSpawnAnimation();
}

float ABaseCharacter::TakeDamage(float DamageAmount, FDamageEvent const & DamageEvent, AController * EventInstigator, AActor * DamageCauser) {DebugPrint(__FILE__, __LINE__);
	const float actualDamage = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);
	MortalityProvider->TakeDamage(actualDamage, EventInstigator, DamageCauser);
	if(EventInstigator && EventInstigator->GetCharacter()) {DebugPrint(__FILE__, __LINE__);
		FVector directionVector = (EventInstigator->GetCharacter()->GetActorLocation() - GetActorLocation());
		directionVector.Normalize();
		OnDirectionalDamageReceived(directionVector, actualDamage, DamageEvent.DamageTypeClass);
	}
	return actualDamage;
}

void ABaseCharacter::StartFire() {DebugPrint(__FILE__, __LINE__);
	DisableSprint();
	sprintAllowed = false;
	ServerStartFire();
}

bool ABaseCharacter::ServerFireOnce_Validate() {DebugPrint(__FILE__, __LINE__);
	return true;
}

void ABaseCharacter::ServerFireOnce_Implementation() {DebugPrint(__FILE__, __LINE__);
	DrawDebugArrow();
	if(!AssertNotNull(CurrentWeapon, GetWorld(), __FILE__, __LINE__))
		return;
	CurrentWeapon->FireOnce();
}

bool ABaseCharacter::ServerStartFire_Validate() {DebugPrint(__FILE__, __LINE__);
	return true;
}

void ABaseCharacter::ServerStartFire_Implementation() {DebugPrint(__FILE__, __LINE__);
	DrawDebugArrow();
	if(!AssertNotNull(CurrentWeapon, GetWorld(), __FILE__, __LINE__))
		return;
	CurrentWeapon->StartFire();
}

void ABaseCharacter::StopFire() {DebugPrint(__FILE__, __LINE__);
	sprintAllowed = true;
	ServerStopFire();
}

bool ABaseCharacter::ServerStopFire_Validate() {DebugPrint(__FILE__, __LINE__);
	return true;
}

void ABaseCharacter::ServerStopFire_Implementation() {DebugPrint(__FILE__, __LINE__);
	if(CurrentWeapon)
		CurrentWeapon->StopFire();
}

void ABaseCharacter::StartAiming() {DebugPrint(__FILE__, __LINE__);
	IsAiming = true;
	CurrentWeapon->StartAiming();
	auto controller = Cast<APlayerControllerBase>(GetController());
	if(controller)
		controller->OnStartAiming(CurrentWeapon);
	ServerStartAiming();
}

bool ABaseCharacter::ServerStartAiming_Validate() {DebugPrint(__FILE__, __LINE__);
	return true;
}

void ABaseCharacter::ServerStartAiming_Implementation() {DebugPrint(__FILE__, __LINE__);
	IsAiming = true;
	MulticastStartAiming();
}

void ABaseCharacter::MulticastStartAiming_Implementation() {DebugPrint(__FILE__, __LINE__);
	IsAiming = true;
	CurrentWeapon->StartAiming();
	auto controller = Cast<APlayerControllerBase>(GetWorld()->GetFirstPlayerController());
	if(controller && controller->SpectatedCharacter == this) {DebugPrint(__FILE__, __LINE__);
		controller->OnStartAiming(CurrentWeapon);
	}
}

void ABaseCharacter::StopAiming() {DebugPrint(__FILE__, __LINE__);
	IsAiming = false;
	if(CurrentWeapon)
		CurrentWeapon->StopAiming();
	auto controller = Cast<APlayerControllerBase>(GetController());
	if(controller)
		controller->OnStopAiming(CurrentWeapon);
	ServerStopAiming();
}

bool ABaseCharacter::ServerStopAiming_Validate() {DebugPrint(__FILE__, __LINE__);
	return true;
}

void ABaseCharacter::ServerStopAiming_Implementation() {DebugPrint(__FILE__, __LINE__);
	IsAiming = false;
	MulticastStopAiming();
}

void ABaseCharacter::MulticastStopAiming_Implementation() {DebugPrint(__FILE__, __LINE__);
	IsAiming = false;
	if(CurrentWeapon)
		CurrentWeapon->StopAiming();
	auto controller = Cast<APlayerControllerBase>(GetWorld()->GetFirstPlayerController());
	if(controller && controller->SpectatedCharacter == this)
		controller->OnStopAiming(CurrentWeapon);
}

bool ABaseCharacter::ServerReload_Validate() {DebugPrint(__FILE__, __LINE__);
	return true;
}

void ABaseCharacter::ServerReload_Implementation() {DebugPrint(__FILE__, __LINE__);
	if(!AssertNotNull(CurrentWeapon, GetWorld(), __FILE__, __LINE__))
		return;
	CurrentWeapon->Reload();
}

bool ABaseCharacter::ServerEquipNextWeapon_Validate() {DebugPrint(__FILE__, __LINE__);
	return true;
}

void ABaseCharacter::ServerEquipNextWeapon_Implementation() {DebugPrint(__FILE__, __LINE__);
	EquipWeapon(WeaponInventory->GetNextWeapon(CurrentWeapon), CurrentWeapon);
}

bool ABaseCharacter::ServerEquipPreviousWeapon_Validate() {DebugPrint(__FILE__, __LINE__);
	return true;
}

void ABaseCharacter::ServerEquipPreviousWeapon_Implementation() {DebugPrint(__FILE__, __LINE__);
	EquipWeapon(WeaponInventory->GetPreviousWeapon(CurrentWeapon), CurrentWeapon);
}

bool ABaseCharacter::ServerEquipSpecificWeapon_Validate(int index) {DebugPrint(__FILE__, __LINE__);
	return true;
}

void ABaseCharacter::ServerEquipSpecificWeapon_Implementation(int index) {DebugPrint(__FILE__, __LINE__);
	ABaseWeapon* newWeapon = WeaponInventory->GetWeapon(index);
	if(newWeapon && CurrentWeapon != newWeapon)
		EquipWeapon(newWeapon, CurrentWeapon);
}

bool ABaseCharacter::ServerEquipRandomWeapon_Validate() {DebugPrint(__FILE__, __LINE__);
	return true;
}

void ABaseCharacter::ServerEquipRandomWeapon_Implementation() {DebugPrint(__FILE__, __LINE__);
	int32 index = FMath::RandHelper(WeaponInventory->GetWeaponNum());
	ABaseWeapon* newWeapon = WeaponInventory->GetWeapon(index);
	if(newWeapon && CurrentWeapon != newWeapon)
		EquipWeapon(newWeapon, CurrentWeapon);
}

void ABaseCharacter::OnRep_CurrentWeapon(ABaseWeapon* OldWeapon) {DebugPrint(__FILE__, __LINE__);
	EquipWeapon(CurrentWeapon, OldWeapon);
}

void ABaseCharacter::EquipWeapon(ABaseWeapon* NewWeapon, ABaseWeapon* OldWeapon) {DebugPrint(__FILE__, __LINE__);
	CurrentWeapon = NewWeapon;

	if(OldWeapon)
		OldWeapon->OnUnequip();
	CurrentWeapon->OnEquip();

	MulticastWeaponChanged(NewWeapon, OldWeapon);
}

void ABaseCharacter::MulticastWeaponChanged_Implementation(ABaseWeapon* newWeapon, ABaseWeapon* oldWeapon) {DebugPrint(__FILE__, __LINE__);
	auto controller = Cast<APlayerControllerBase>(GetController());
	if(controller)
		controller->OnWeaponChanged(newWeapon, oldWeapon);
	controller = Cast<APlayerControllerBase>(GetWorld()->GetFirstPlayerController());
	if(controller && controller->SpectatedCharacter == this)
		controller->OnWeaponChanged(newWeapon, oldWeapon);
}

void ABaseCharacter::DrawDebugArrow() {DebugPrint(__FILE__, __LINE__);
	if(Cast<UInflectionPointCheatManager>(GetWorld()->GetFirstPlayerController()->CheatManager) &&
		Cast<UInflectionPointCheatManager>(GetWorld()->GetFirstPlayerController()->CheatManager)->IsCharacterDebugArrowsEnabled) {DebugPrint(__FILE__, __LINE__);
		FRotator cameraRot = FirstPersonCameraComponent->GetComponentRotation();
		FVector cameraDirectionVector = cameraRot.Vector() * 15 + GetTransform().GetLocation();

		DrawDebugDirectionalArrow(GetWorld(), GetTransform().GetLocation(), cameraDirectionVector, DebugArrowColor.B, DebugArrowColor, true, -1, 0, 0.5f);
	}
}

void ABaseCharacter::MulticastOnDeath_Implementation() {DebugPrint(__FILE__, __LINE__);
	// Play random Death Animation
	if(DeathAnimations.Num() == 0) {DebugPrint(__FILE__, __LINE__);
		UE_LOG(LogTemp, Warning, TEXT("Warning: No Death Animations set for this character!"));
	} else {DebugPrint(__FILE__, __LINE__);
		int index = FMath::RandRange(0, DeathAnimations.Num() - 1);
		Mesh3P->PlayAnimation(DeathAnimations[index], false);
	}

	// Disable all input
	DisableInput((APlayerController*)GetController());
	DisableComponentsSimulatePhysics();

	GetCapsuleComponent()->SetCollisionProfileName(FName("DeadCharacter"));
}

void ABaseCharacter::MulticastPlay3PAnimation_Implementation(UAnimationAsset* animation) {DebugPrint(__FILE__, __LINE__);
	Mesh3P->PlayAnimation(animation, false);
}

void ABaseCharacter::ClientOnDeath_Implementation() {DebugPrint(__FILE__, __LINE__);
	Mesh3P->SetOwnerNoSee(false);
	Mesh1P->SetVisibility(false, true);
	if(!AssertNotNull(GetController(), GetWorld(), __FILE__, __LINE__))
		return;
	FRotator rot = GetController()->GetControlRotation();
	GetController()->SetControlRotation(FRotator(-30, rot.Yaw, rot.Roll));
}

void ABaseCharacter::MoveForward(float value) {DebugPrint(__FILE__, __LINE__);
	if(value != 0.0f) {DebugPrint(__FILE__, __LINE__);
		AddMovementInput(GetActorForwardVector(), value);
	}

	if(ShouldStartSprinting(value)) {DebugPrint(__FILE__, __LINE__);
		StartSprinting();
		ServerStartSprinting();
	} else if(ShouldStopSprinting(value)) {DebugPrint(__FILE__, __LINE__);
		StopSprinting();
		ServerStopSprinting();
	}
}

void ABaseCharacter::MoveRight(float value) {DebugPrint(__FILE__, __LINE__);
	if(value != 0.0f) {DebugPrint(__FILE__, __LINE__);
		AddMovementInput(GetActorRightVector(), value);
	}
}

void ABaseCharacter::TurnAtRate(float rate) {DebugPrint(__FILE__, __LINE__);
	// calculate delta for this frame from the rate information
	AddControllerYawInput(rate);
}

void ABaseCharacter::LookUpAtRate(float rate) {DebugPrint(__FILE__, __LINE__);
	// calculate delta for this frame from the rate information
	AddControllerPitchInput(rate);
	ServerUpdateCameraPitch(FirstPersonCameraComponent->GetComponentRotation().Pitch);
	MulticastUpdateCameraPitch(FirstPersonCameraComponent->GetComponentRotation().Pitch);
}

bool ABaseCharacter::ServerUpdateCameraPitch_Validate(float pitch) {DebugPrint(__FILE__, __LINE__);
	return true;
}

void ABaseCharacter::ServerUpdateCameraPitch_Implementation(float pitch) {DebugPrint(__FILE__, __LINE__);
	auto currentRot = FirstPersonCameraComponent->GetComponentRotation();
	currentRot.Pitch = pitch;
	currentRot.Roll = 0;
	currentRot.Yaw = 0;
	FirstPersonCameraComponent->SetRelativeRotation(currentRot);
}

void ABaseCharacter::MulticastUpdateCameraPitch_Implementation(float pitch) {DebugPrint(__FILE__, __LINE__);
	auto currentRot = FirstPersonCameraComponent->GetComponentRotation();
	currentRot.Pitch = pitch;
	currentRot.Roll = 0;
	currentRot.Yaw = 0;
	FirstPersonCameraComponent->SetRelativeRotation(currentRot);
}

void ABaseCharacter::EnableSprint() {DebugPrint(__FILE__, __LINE__);
	sprintEnabled = true;
}

void ABaseCharacter::DisableSprint() {DebugPrint(__FILE__, __LINE__);
	sprintEnabled = false;
}

bool ABaseCharacter::ShouldStartSprinting(float ForwardMovement) {DebugPrint(__FILE__, __LINE__);
	return sprintAllowed && sprintEnabled && ForwardMovement > 0 && GetVelocity().Size() <= WalkSpeed;
}

bool ABaseCharacter::ShouldStopSprinting(float ForwardMovement) {DebugPrint(__FILE__, __LINE__);
	if(GetVelocity().Size() <= WalkSpeed) {DebugPrint(__FILE__, __LINE__);
		return false;
	}
	return !sprintAllowed || !sprintEnabled || ForwardMovement <= 0;
}

void ABaseCharacter::StartSprinting() {DebugPrint(__FILE__, __LINE__);
	GetCharacterMovement()->MaxWalkSpeed = SprintSpeed;
}

void ABaseCharacter::StopSprinting() {DebugPrint(__FILE__, __LINE__);
	GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;
}

bool ABaseCharacter::ServerStartSprinting_Validate() {DebugPrint(__FILE__, __LINE__);
	return true;
}

void ABaseCharacter::ServerStartSprinting_Implementation() {DebugPrint(__FILE__, __LINE__);
	StartSprinting();
}

bool ABaseCharacter::ServerStopSprinting_Validate() {DebugPrint(__FILE__, __LINE__);
	return true;
}

void ABaseCharacter::ServerStopSprinting_Implementation() {DebugPrint(__FILE__, __LINE__);
	StopSprinting();
}

bool ABaseCharacter::IsAlive() {DebugPrint(__FILE__, __LINE__);
	return MortalityProvider && MortalityProvider->IsAlive();
}

bool ABaseCharacter::IsInSameTeamAsLocalPlayer() {DebugPrint(__FILE__, __LINE__);
	auto localPlayerState = Cast<ATDMPlayerStateBase>(GetWorld()->GetFirstPlayerController()->PlayerState);
	if(!localPlayerState)
		return false;
	return localPlayerState->Team == CharacterInfoProvider->GetCharacterInfo().Team;
}



void ABaseCharacter::UpdateCharacterNameTag() {DebugPrint(__FILE__, __LINE__);
	if(!GetWorld()->GetFirstPlayerController()->PlayerState || !IsInSameTeamAsLocalPlayer() || !IsAlive()) {DebugPrint(__FILE__, __LINE__);
		CharacterNameTag->SetVisibility(false);
		return;
	}
	CharacterNameTag->SetVisibility(true);
	CharacterNameTag->SetWorldRotation(FRotator(0, 0, 0));
	CharacterNameTag->SetText(FText::FromString(CharacterInfoProvider->GetCharacterInfo().PlayerName));
}