// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "InflectionPointCharacter.h"
#include "ReplayControlledFPSCharacter.generated.h"


/**
 *
 */
UCLASS()
class INFLECTIONPOINT_API AReplayControlledFPSCharacter : public AInflectionPointCharacter {
	GENERATED_BODY()

public:

	enum Key {
		LMB,
		RMB,
		SPACE
	};

	virtual void BeginPlay() override;

	void StartReplay(TArray<float> inputs, TArray<float> yaws, TArray<float> pitches, TArray<float> moveForwards, TArray<float> moveRights);

	UFUNCTION()
	void PressKey(float key);

	UFUNCTION()
	void ApplyYaw(float value);

	UFUNCTION()
	void ApplyPitch(float value);

	UFUNCTION()
	void ReplayMoveForward(float value);

	UFUNCTION()
	void ReplayMoveRight(float value);

private:
	TArray<float> recordedInput;
	bool playing = false;
	float remainingTime = 0;


};
