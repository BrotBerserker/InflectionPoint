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

// Called every frame
void AReplayControlledFPSCharacter::Tick(float DeltaTime) {
	if(!IsReplaying)
		return;

	passedTime += DeltaTime;


	for(; RecordData[replayIndex].Timestamp <= passedTime; replayIndex++) {
		auto recordDataStep = RecordData[replayIndex];
		for(auto &item : recordDataStep.ButtonsPressed) {
			if(!pressedButtons.Contains(item)) {
				PressKey(recordDataStep.CapsuleYaw, recordDataStep.CameraPitch, item);
				pressedButtons.Add(item);
			}
		}
		for(auto &item : pressedButtons) {
			if(!recordDataStep.ButtonsPressed.Contains(item)) {
				ReleaseKey(recordDataStep.CapsuleYaw, recordDataStep.CameraPitch, item);
				pressedButtons.Remove(item);
			}
		}
	}
}

void AReplayControlledFPSCharacter::StartReplay(TArray<FRecordedPlayerState> recordData) {
	RecordData = recordData;
	IsReplaying = true;
	replayIndex = 0;
}

// Helper Method 
void AReplayControlledFPSCharacter::StartTimerForKeyChanged(TPair<FString, TArray<TTuple<float, float, float>>> & element, FString timerFunction) {
	while(element.Value.Num() > 0) {
		StartTimer(this, GetWorld(), timerFunction,
			element.Value.Last().Get<0>() + 0.0333333f, // time
			element.Value.Last().Get<1>(), // yaw
			element.Value.Last().Get<2>(), // pitch
			element.Key); // Key
		element.Value.RemoveAt(element.Value.Num() - 1);
	}
}

void AReplayControlledFPSCharacter::PressKey(float yaw, float pitch, FString key) {
	// set rotation
	ApplyYaw(yaw);
	ApplyPitch(pitch);

	// press key
	if(key == "Jump") {
		Jump();
	} else if(key == "Fire") {
		OnFire();
	} else if(key == "DEBUG_Fire") {
		OnDebugFire();
	} else if(key == "MoveForward") {
		isForwardPressed = true;
		ReplayMoveForward(1);
	} else if(key == "MoveBackward") {
		isBackwordPressed = true;
		ReplayMoveForward(-1);
	} else if(key == "MoveLeft") {
		isLeftPressed = true;
		ReplayMoveRight(-1);
	} else if(key == "MoveRight") {
		isRightPressed = true;
		ReplayMoveRight(1);
	}
}

void AReplayControlledFPSCharacter::ReleaseKey(float yaw, float pitch, FString key) {
	// set rotation
	ApplyYaw(yaw);
	ApplyPitch(pitch);

	// release key
	if(key == "MoveForward") {
		isForwardPressed = false;
	} else if(key == "MoveBackward") {
		isBackwordPressed = false;
	} else if(key == "MoveLeft") {
		isLeftPressed = false;
	} else if(key == "MoveRight") {
		isRightPressed = false;
	}
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

void AReplayControlledFPSCharacter::ReplayMoveForward(float value) {
	MoveForward(value);
}

void AReplayControlledFPSCharacter::ReplayMoveRight(float value) {
	MoveRight(value);
}


