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

void AReplayControlledFPSCharacter::StartReplay(TArray<FRecordedPlayerState> recordData) {
	RecordData = recordData;
	IsReplaying = true;
	ReplayIndex = 0;
}

void AReplayControlledFPSCharacter::StopReplay() {
	IsReplaying = false;
	ReplayIndex = 0;
}

// Called every frame
void AReplayControlledFPSCharacter::Tick(float deltaTime) {
	if(!IsReplaying)
		return;
	PassedTime += deltaTime;
	PassedTimeSinceLastCorrection += deltaTime;

	// Correct position 
	if(ReplayIndex != 0 && PositionCorrectionInterval >= 0 && PassedTimeSinceLastCorrection > PositionCorrectionInterval)
		TryCorrectPosition(RecordData[ReplayIndex-1].Position);

	UpdatePressedKeys();

	// Call Hold for all currently pressed buttons
	for(auto &key : PressedKeys) {
		HoldKey(key);
	}

	// stop replay when end of RecordData reached
	if(ReplayIndex >= RecordData.Num())
		StopReplay();
}

void AReplayControlledFPSCharacter::UpdatePressedKeys() {
	// iterate through all record data since last tick until now
	for(; ReplayIndex < RecordData.Num() && RecordData[ReplayIndex].Timestamp <= PassedTime; ReplayIndex++) {
		if(ReplayIndex != 0) { // Update Rotation (-1 because unreal ^^)
			ApplyYaw(RecordData[ReplayIndex - 1].CapsuleYaw);
			ApplyPitch(RecordData[ReplayIndex - 1].CameraPitch);
		}
		auto recordDataStep = RecordData[ReplayIndex];
		UpdatePressedKeys(recordDataStep);
		UpdateReleasedKeys(recordDataStep);
	}
}

void AReplayControlledFPSCharacter::UpdatePressedKeys(FRecordedPlayerState &recordDataStep) {
	for(auto &item : recordDataStep.PressedKeys) {
		if(!PressedKeys.Contains(item)) {
			PressKey(item);
			PressedKeys.Add(item);
		}
	}
}

void AReplayControlledFPSCharacter::UpdateReleasedKeys(FRecordedPlayerState &recordDataStep) {
	for(int i = 0; i < PressedKeys.Num(); i++) {
		auto item = PressedKeys[i];
		if(!recordDataStep.PressedKeys.Contains(item)) {
			ReleaseKey(item);
			PressedKeys.Remove(item);
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
		ReplayMoveForward(1);
	} else if(key == "MoveBackward") {
		ReplayMoveForward(-1);
	} else if(key == "MoveLeft") {
		ReplayMoveRight(-1);
	} else if(key == "MoveRight") {
		ReplayMoveRight(1);
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
		PassedTimeSinceLastCorrection = 0;
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

void AReplayControlledFPSCharacter::ReplayMoveForward(float value) {
	MoveForward(value);
}

void AReplayControlledFPSCharacter::ReplayMoveRight(float value) {
	MoveRight(value);
}