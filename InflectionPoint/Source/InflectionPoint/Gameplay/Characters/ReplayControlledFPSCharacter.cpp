// Fill out your copyright notice in the Description page of Project Settings.

#include "InflectionPoint.h"
#include "ReplayControlledFPSCharacter.h"
#include "Utils/TimerFunctions.h"


// Sets default values for this component's properties
AReplayControlledFPSCharacter::AReplayControlledFPSCharacter() {
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryActorTick.bCanEverTick = true;
}

void AReplayControlledFPSCharacter::BeginPlay() {
	Super::BeginPlay();
	//PrimaryActorTick.bCanEverTick = true;
}

void AReplayControlledFPSCharacter::StartReplay() {
	replayIndex = 0;
	isReplaying = true;
}

void AReplayControlledFPSCharacter::SetReplayData(TArray<FRecordedPlayerState> RecordData) {
	recordData = RecordData;
}

void AReplayControlledFPSCharacter::StopReplay() {
	isReplaying = false;
	replayIndex = 0;
}

void AReplayControlledFPSCharacter::Tick(float deltaTime) {
	if(!isReplaying)
		return;
	passedTime += deltaTime;
	passedTimeSinceLastCorrection += deltaTime;

	// Correct position 
	bool correctPosition = CurrentPositionShouldBeCorrected();
	if(correctPosition)
		CorrectPosition(recordData[replayIndex - 1].Position);

	// Draw debug sphere
	if(CreateDebugCorrectionSpheres)
		DrawDebugSphereAtCurrentPosition(correctPosition);

	UpdatePressedKeys();

	// Call Hold for all currently pressed buttons
	for(auto &key : pressedKeys) {
		HoldKey(key);
	}

	// stop replay when end of recordData reached
	if(replayIndex >= recordData.Num())
		StopReplay();
}

void AReplayControlledFPSCharacter::UpdatePressedKeys() {
	// iterate through all record data since last tick until now
	for(; replayIndex < recordData.Num() && recordData[replayIndex].Timestamp <= passedTime; replayIndex++) {
		if(replayIndex != 0) { // Update Rotation (-1 because unreal ^^)
			ApplyYaw(recordData[replayIndex - 1].CapsuleYaw);
			ApplyPitch(recordData[replayIndex - 1].CameraPitch);
		}
		auto recordDataStep = recordData[replayIndex];
		UpdatePressedKeys(recordDataStep);
		UpdateReleasedKeys(recordDataStep);
	}
}

void AReplayControlledFPSCharacter::UpdatePressedKeys(FRecordedPlayerState &recordDataStep) {
	for(auto &item : recordDataStep.PressedKeys) {
		if(!pressedKeys.Contains(item)) {
			PressKey(item);
			pressedKeys.Add(item);
		}
	}
}

void AReplayControlledFPSCharacter::UpdateReleasedKeys(FRecordedPlayerState &recordDataStep) {
	for(int i = 0; i < pressedKeys.Num(); i++) {
		auto item = pressedKeys[i];
		if(!recordDataStep.PressedKeys.Contains(item)) {
			ReleaseKey(item);
			pressedKeys.Remove(item);
			i--;
		}
	}
}

void AReplayControlledFPSCharacter::PressKey(FString key) {
	if(key == "Jump") {
		Jump();
	} else if(key == "Fire") {
		if(replayIndex > 0) {
			CorrectPosition(recordData[replayIndex - 1].Position);
		}
		OnFire();
	} else if(key == "DEBUG_Fire") {
		if(replayIndex > 0) {
			CorrectPosition(recordData[replayIndex - 1].Position);
		}
		OnDebugFire();
	}
}

void AReplayControlledFPSCharacter::HoldKey(FString key) {
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

void AReplayControlledFPSCharacter::ReleaseKey(FString key) {
	// ...
}

void AReplayControlledFPSCharacter::ApplyYaw(float value) {
	FRotator rot = GetCapsuleComponent()->GetComponentRotation();
	rot.Yaw = value;
	rot.Roll = 0;
	rot.Pitch = 0;
	GetCapsuleComponent()->SetWorldRotation(rot);

	FRotator rot2 = FirstPersonCameraComponent->GetComponentRotation();
	rot2.Yaw = value;
	FirstPersonCameraComponent->SetWorldRotation(rot2);
}

void AReplayControlledFPSCharacter::ApplyPitch(float value) {
	FRotator rot = FirstPersonCameraComponent->GetComponentRotation();
	rot.Pitch = value;
	rot.Roll = 0;
	FirstPersonCameraComponent->SetWorldRotation(rot);
}

bool AReplayControlledFPSCharacter::CurrentPositionShouldBeCorrected() {
	if(replayIndex == 0)
		return false;

	if(PositionCorrectionInterval < 0)
		return false;

	if(passedTimeSinceLastCorrection <= PositionCorrectionInterval)
		return false;

	if(CorrectionRadius < 0)
		return true;

	FVector actualPosition = GetTransform().GetLocation();
	FVector correctPosition = recordData[replayIndex - 1].Position;

	return FVector::Dist(actualPosition, correctPosition) <= CorrectionRadius;
}

void AReplayControlledFPSCharacter::DrawDebugSphereAtCurrentPosition(bool positionHasBeenCorrected) {
	FColor sphereColor = positionHasBeenCorrected ? DebugHitColor : DebugMissColor;
	DrawDebugSphere(GetWorld(), GetTransform().GetLocation(), CorrectionRadius, 8, sphereColor, true);
}

void AReplayControlledFPSCharacter::CorrectPosition(FVector correctPosition) {
	SetActorLocation(correctPosition);
	passedTimeSinceLastCorrection = 0;
}