// Fill out your copyright notice in the Description page of Project Settings.

#include "InflectionPoint.h"
#include "ReplayControlledFPSCharacter.h"


void AReplayControlledFPSCharacter::BeginPlay() {
	Super::BeginPlay();
	PrimaryActorTick.bCanEverTick = true;
}

void AReplayControlledFPSCharacter::StartReplay(TArray<float> inputs, TArray<float> moveForwards, TArray<float> moveRights) {
	//TArray<TPair<FKey, TTuple<float, float, float>>> asd;
	while(inputs.Num() > 0) {
		float key = inputs.Last();
		inputs.RemoveAt(inputs.Num() - 1);
		float pitch = inputs.Last();
		inputs.RemoveAt(inputs.Num() - 1);
		float yaw = inputs.Last();
		inputs.RemoveAt(inputs.Num() - 1);
		float wait = inputs.Last();
		inputs.RemoveAt(inputs.Num() - 1);

		StartTimer("PressKey", wait, yaw, pitch, key);
	}

	while(moveForwards.Num() > 0) {
		float forward = moveForwards.Last();
		moveForwards.RemoveAt(moveForwards.Num() - 1);
		float wait = moveForwards.Last();
		moveForwards.RemoveAt(moveForwards.Num() - 1);

		StartTimer("ReplayMoveForward", wait, forward);
	}

	while(moveRights.Num() > 0) {
		float right = moveRights.Last();
		moveRights.RemoveAt(moveRights.Num() - 1);
		float wait = moveRights.Last();
		moveRights.RemoveAt(moveRights.Num() - 1);

		StartTimer("ReplayMoveRight", wait, right);
	}
}

template <typename... VarTypes>
void AReplayControlledFPSCharacter::StartTimer(FString function, float wait, VarTypes... vars) {
	FTimerHandle TimerHandle;
	FTimerDelegate TimerDel;
	TimerDel.BindUFunction(this, FName(*function), std::forward<VarTypes>(vars)...);
	GetWorld()->GetTimerManager().SetTimer(TimerHandle, TimerDel, wait, false, wait);
}

void AReplayControlledFPSCharacter::PressKey(float yaw, float pitch, float key) {
	// set rotation
	ApplyYaw(yaw);
	ApplyPitch(pitch);

	// press key
	if(key == Key::SPACE) {
		Jump();
	} else if(key == Key::LMB) {
		OnFire();
	} else if(key == Key::RMB) {
		OnDebugFire();
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


