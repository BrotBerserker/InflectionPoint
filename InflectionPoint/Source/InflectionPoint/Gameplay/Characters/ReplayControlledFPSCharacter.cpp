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

void AReplayControlledFPSCharacter::StartReplay(TArray<FRecordedPlayerState> RecordData) {
	recordData = RecordData;
	isReplaying = true;
	replayIndex = 0;
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
	if(replayIndex != 0 && PositionCorrectionInterval >= 0 && passedTimeSinceLastCorrection > PositionCorrectionInterval)
		TryCorrectPosition(recordData[replayIndex - 1].Position);

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
			ApplyPitch(recordData[replayIndex].CameraPitch);
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
		OnFire();
	} else if(key == "DEBUG_Fire") {
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

	FRotator rot2 = GetFirstPersonCameraComponent()->GetComponentRotation();
	rot2.Yaw = value;
	GetFirstPersonCameraComponent()->SetWorldRotation(rot2);
}

void AReplayControlledFPSCharacter::ApplyPitch(float value) {
	FRotator rot = GetFirstPersonCameraComponent()->GetComponentRotation();
	rot.Pitch = value;
	rot.Roll = 0;
	GetFirstPersonCameraComponent()->SetWorldRotation(rot);
}

bool AReplayControlledFPSCharacter::IsAtProperPosition(FVector correctPosition) {
	FVector actualPosition = GetTransform().GetLocation();
	return CorrectionRadius < 0 || FVector::Dist(actualPosition, correctPosition) <= CorrectionRadius;
}

bool AReplayControlledFPSCharacter::TryCorrectPosition(FVector correctPosition) {
	if(IsAtProperPosition(correctPosition)) {
		SetActorLocation(correctPosition);
		passedTimeSinceLastCorrection = 0;
		if(CreateDebugCorrectionSpheres) {
			DrawDebugSphere(GetWorld(), GetTransform().GetLocation(), CorrectionRadius, 8, DebugHitColor, true);
		}
		return true;
	}
	if(CreateDebugCorrectionSpheres) {
		DrawDebugSphere(GetWorld(), GetTransform().GetLocation(), CorrectionRadius, 8, DebugMissColor, true);
	}
	return false;
}