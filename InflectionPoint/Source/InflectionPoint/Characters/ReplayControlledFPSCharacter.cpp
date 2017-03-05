// Fill out your copyright notice in the Description page of Project Settings.

#include "InflectionPoint.h"
#include "ReplayControlledFPSCharacter.h"


void AReplayControlledFPSCharacter::BeginPlay() {
	Super::BeginPlay();
	PrimaryActorTick.bCanEverTick = true;
}

void AReplayControlledFPSCharacter::StartReplay(TArray<float> inputs, TArray<float> yaws, TArray<float> pitches, TArray<float> moveForwards, TArray<float> moveRights) {
	while(inputs.Num() > 0) {
		float key = inputs.Last();
		inputs.RemoveAt(inputs.Num() - 1);
		float wait = inputs.Last();
		inputs.RemoveAt(inputs.Num() - 1);

		//UE_LOG(LogTemp, Warning, TEXT("Waiting %f and then pressing %f!"), wait, key);

		FTimerHandle TimerHandle;
		FTimerDelegate TimerDel;
		TimerDel.BindUFunction(this, FName("PressKey"), key);

		GetWorld()->GetTimerManager().SetTimer(TimerHandle, TimerDel, wait, false, wait);
	}
	while(moveForwards.Num() > 0) {
		float forward = moveForwards.Last();
		moveForwards.RemoveAt(moveForwards.Num() - 1);
		float wait = moveForwards.Last();
		moveForwards.RemoveAt(moveForwards.Num() - 1);

		//UE_LOG(LogTemp, Warning, TEXT("Waiting %f and then applying yaw: %f!"), wait, yaw);

		FTimerHandle TimerHandle;
		FTimerDelegate TimerDel;
		TimerDel.BindUFunction(this, FName("ReplayMoveForward"), forward);

		GetWorld()->GetTimerManager().SetTimer(TimerHandle, TimerDel, wait, false, wait);
	}
	while(moveRights.Num() > 0) {
		float right = moveRights.Last();
		moveRights.RemoveAt(moveRights.Num() - 1);
		float wait = moveRights.Last();
		moveRights.RemoveAt(moveRights.Num() - 1);

		//UE_LOG(LogTemp, Warning, TEXT("Waiting %f and then applying yaw: %f!"), wait, yaw);

		FTimerHandle TimerHandle;
		FTimerDelegate TimerDel;
		TimerDel.BindUFunction(this, FName("ReplayMoveRight"), right);

		GetWorld()->GetTimerManager().SetTimer(TimerHandle, TimerDel, wait, false, wait);
	}
}

void AReplayControlledFPSCharacter::PressKey(float key) {
	//UE_LOG(LogTemp, Warning, TEXT("Pressing %f!"), key);
	if(key == Key::SPACE) {
		Jump();
	} else if(key == Key::LMB) {
		OnFire();
	}
}

void AReplayControlledFPSCharacter::ApplyYaw(float value) {
	//UE_LOG(LogTemp, Warning, TEXT("Applying yaw: %f!"), value);
	AddControllerYawInput(value);
}

void AReplayControlledFPSCharacter::ApplyPitch(float value) {
	//AddControllerPitchInput(value);
	AddActorLocalRotation(FQuat(0, 0, value, 0));
}

void AReplayControlledFPSCharacter::ReplayMoveForward(float value) {
	MoveForward(value);
}

void AReplayControlledFPSCharacter::ReplayMoveRight(float value) {
	MoveRight(value);
}


