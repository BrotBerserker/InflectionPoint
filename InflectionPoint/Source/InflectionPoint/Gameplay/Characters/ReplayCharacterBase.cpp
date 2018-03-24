// Fill out your copyright notice in the Description page of Project Settings.

#include "InflectionPoint.h"
#include "ReplayCharacterBase.h"
#include "Gameplay/Controllers/AIControllerBase.h"
#include "Utils/TimerFunctions.h"


// Sets default values for this component's properties
AReplayCharacterBase::AReplayCharacterBase() {
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryActorTick.bCanEverTick = true;
}

void AReplayCharacterBase::BeginPlay() {
	Super::BeginPlay();
	//PrimaryActorTick.bCanEverTick = true;
}

bool AReplayCharacterBase::IsReadyForInitialization() {
	if(!GetController()) {
		return false;
	}
	APlayerController* owningController = Cast<AAIControllerBase>(GetController())->OwningPlayerController;
	if(!owningController) {
		return false;
	}
	if(!owningController->PlayerState) {
		return false;
	}
	return true;
}

void AReplayCharacterBase::Initialize() {
	APlayerController* owningController = Cast<AAIControllerBase>(GetController())->OwningPlayerController;
	AssertNotNull(owningController, GetWorld(), __FILE__, __LINE__);
	MulticastApplyPlayerColor(Cast<ATDMPlayerStateBase>(owningController->PlayerState));
	MulticastShowSpawnAnimation();
}

void AReplayCharacterBase::StartReplay() {
	replayIndex = 0;
	isReplaying = true;
}

void AReplayCharacterBase::SetReplayData(TArray<FRecordedPlayerState> RecordData) {
	recordData = RecordData;
}

void AReplayCharacterBase::StopReplay() {
	isReplaying = false;
	replayIndex = 0;
}

void AReplayCharacterBase::Tick(float deltaTime) {
	Super::Tick(deltaTime);
	if(!isReplaying)
		return;
	passedTime += deltaTime;
	passedTimeSinceLastCorrection += deltaTime;

	// Correct position 
	bool shouldCorrectPosition = CurrentPositionShouldBeCorrected();
	if(shouldCorrectPosition)
		CorrectPosition(recordData[replayIndex].Position);

	// Draw debug sphere
	if(CreateDebugCorrectionSpheres)
		DrawDebugSphereAtCurrentPosition(shouldCorrectPosition);

	UpdateKeys();

	// Call Hold for all currently pressed buttons
	for(auto &key : pressedKeys) {
		HoldKey(key);
	}

	// stop replay when end of recordData reached
	if(replayIndex >= recordData.Num())
		StopReplay();
}

void AReplayCharacterBase::UpdateKeys() {
	// iterate through all record data since last tick until now
	for(; replayIndex < recordData.Num() && recordData[replayIndex].Timestamp <= passedTime; replayIndex++) {
		UpdateRotation();
		auto recordDataStep = recordData[replayIndex];
		UpdatePressedKeys(recordDataStep);
		UpdateReleasedKeys(recordDataStep);
	}
}

void AReplayCharacterBase::UpdateRotation() {
	if(replayIndex == 0) {
		return;
	}
	// Update Rotation
	ApplyYaw(recordData[replayIndex].CapsuleYaw);
	ApplyPitch(recordData[replayIndex].CameraPitch);
}

void AReplayCharacterBase::UpdatePressedKeys(FRecordedPlayerState &recordDataStep) {
	for(auto &item : recordDataStep.PressedKeys) {
		PressKey(item);
		pressedKeys.Add(item);
	}
}

void AReplayCharacterBase::UpdateReleasedKeys(FRecordedPlayerState &recordDataStep) {
	for(auto &item : recordDataStep.ReleasedKeys) {
		ReleaseKey(item);
		pressedKeys.Remove(item);
	}
}

void AReplayCharacterBase::PressKey(FString key) {
	if(key == "Jump") {
		Jump();
	} else if(key == "Aim") {
		StartAiming();
	} else if(key == "Sprint") {
		EnableSprint();
	} else if(key == "WeaponFired") {
		CurrentWeapon->Fire();
	} else if(key == "Reload") {
		CurrentWeapon->Reload();
	} else if(key == "EquipNextWeapon") {
		ServerEquipNextWeapon();
	} else if(key == "EquipPreviousWeapon") {
		ServerEquipPreviousWeapon();
	} else if(key.Contains("EquipSpecificWeapon")) {
		auto str = FString(key); // to not alter string
		str.RemoveFromStart("EquipSpecificWeapon");
		int index = FCString::Atoi(*str);
		ServerEquipSpecificWeapon(index);
	}
}

void AReplayCharacterBase::HoldKey(FString key) {
	if(key == "MoveForward") {
		MoveForward(1);
	} else if(key == "MoveBackward") {
		MoveForward(-1);
	} else if(key == "MoveLeft") {
		MoveRight(-1);
	} else if(key == "MoveRight") {
		MoveRight(1);
	}
}

void AReplayCharacterBase::ReleaseKey(FString key) {
	if(key == "Sprint") {
		DisableSprint();
	} else if(key == "Aim") {
		StopAiming();
	}
}

void AReplayCharacterBase::ApplyYaw(float value) {
	FRotator rot = GetCapsuleComponent()->GetComponentRotation();
	rot.Yaw = value;
	rot.Roll = 0;
	rot.Pitch = 0;
	GetCapsuleComponent()->SetWorldRotation(rot);

	FRotator rot2 = FirstPersonCameraComponent->GetComponentRotation();
	rot2.Yaw = value;
	FirstPersonCameraComponent->SetWorldRotation(rot2);
}

void AReplayCharacterBase::ApplyPitch(float value) {
	FRotator rot = FirstPersonCameraComponent->GetComponentRotation();
	rot.Pitch = value;
	rot.Roll = 0;
	FirstPersonCameraComponent->SetWorldRotation(rot);
}

bool AReplayCharacterBase::CurrentPositionShouldBeCorrected() {
	if(replayIndex == 0)
		return false;

	return CorrectPositions;
}

bool AReplayCharacterBase::CurrentPositionIsInCorrectionRadius(float radius) {
	if(radius < 0)
		return true;

	FVector actualPosition = GetTransform().GetLocation();
	FVector correctPosition = recordData[replayIndex - 1].Position;

	return FVector::Dist(actualPosition, correctPosition) <= radius;
}

void AReplayCharacterBase::DrawDebugSphereAtCurrentPosition(bool positionHasBeenCorrected) {
	FColor sphereColor = positionHasBeenCorrected ? DebugHitColor : DebugMissColor;
	DrawDebugSphere(GetWorld(), GetTransform().GetLocation(), CorrectionRadius, 8, sphereColor, true);
}

void AReplayCharacterBase::CorrectPosition(FVector correctPosition) {
	SetActorLocation(correctPosition);
	passedTimeSinceLastCorrection = 0;
}