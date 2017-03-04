// Fill out your copyright notice in the Description page of Project Settings.

#include "InflectionPoint.h"
#include "ReplayControlledFPSCharacter.h"


void AReplayControlledFPSCharacter::BeginPlay() {
	PrimaryActorTick.bCanEverTick = true;
}

void AReplayControlledFPSCharacter::StartReplay(TArray<float> inputs) {
	while(inputs.Num() > 0) {
		float key = inputs.Last();
		inputs.RemoveAt(inputs.Num() - 1);
		float wait = -inputs.Last();
		inputs.RemoveAt(inputs.Num() - 1);

		UE_LOG(LogTemp, Warning, TEXT("Waiting %f and then pressing %f!"), wait, key);

		FTimerDelegate TimerDel;
		FTimerHandle TimerHandle;
		TimerDel.BindUFunction(this, FName("PressKey"), key);

		GetWorld()->GetTimerManager().SetTimer(TimerHandle, TimerDel, wait, false, wait);
	}
}

void AReplayControlledFPSCharacter::PressKey(float key) {
	UE_LOG(LogTemp, Warning, TEXT("Pressing %f!"), key);
	if(key == Key::SPACE) {
		Jump();
	} else if(key == Key::LMB) {
		OnFire();
	}
}


