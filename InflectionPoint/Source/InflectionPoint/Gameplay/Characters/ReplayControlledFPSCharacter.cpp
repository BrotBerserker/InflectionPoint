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
	if(isForwardPressed)
		ReplayMoveForward(1);
	if(isBackwordPressed)
		ReplayMoveForward(-1);
	if(isRightPressed)
		ReplayMoveRight(1);
	if(isLeftPressed)
		ReplayMoveRight(-1);

}

void AReplayControlledFPSCharacter::StartReplay(TMap<FString, TArray<TTuple<float, float, float>>> keysPressed, TMap<FString, TArray<TTuple<float, float, float>>> keysReleased) {
	for(auto& element : keysPressed) {
		StartTimerForKeyChanged(element, "PressKey");
	}
	for(auto& element : keysReleased) {
		StartTimerForKeyChanged(element, "ReleaseKey");
	}
	/*while(inputs.Num() > 0) {
		FKey key = inputs.Last().Key;
		float wait = inputs.Last().Value.Get<0>();
		float yaw = inputs.Last().Value.Get<1>();
		float pitch = inputs.Last().Value.Get<2>();
		inputs.RemoveAt(inputs.Num() - 1);

		StartTimer(this, GetWorld(), "PressKey", wait, yaw, pitch, key);
	}*/

	/*while(moveForwards.Num() > 0) {
		float forward = moveForwards.Last();
		moveForwards.RemoveAt(moveForwards.Num() - 1);
		float wait = moveForwards.Last();
		moveForwards.RemoveAt(moveForwards.Num() - 1);

		StartTimer(this, GetWorld(), "ReplayMoveForward", wait, forward);
	}

	while(moveRights.Num() > 0) {
		float right = moveRights.Last();
		moveRights.RemoveAt(moveRights.Num() - 1);
		float wait = moveRights.Last();
		moveRights.RemoveAt(moveRights.Num() - 1);

		StartTimer(this, GetWorld(), "ReplayMoveRight", wait, right);
	}*/
}

// Helper Method 
void AReplayControlledFPSCharacter::StartTimerForKeyChanged(TPair<FString, TArray<TTuple<float, float, float>>> & element, FString timerFunction) {
	while(element.Value.Num() > 0) {
		StartTimer(this, GetWorld(), timerFunction,
			element.Value.Last().Get<0>(), // time
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
	} else if(key == "MoveBackward") {
		isBackwordPressed = true;
	} else if(key == "MoveLeft") {
		isLeftPressed = true;
	} else if(key == "MoveRight") {
		isRightPressed = true;
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


