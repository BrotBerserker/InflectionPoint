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
		SPACE
	};

	virtual void BeginPlay() override;

	void StartReplay(TArray<float> inputs);

	UFUNCTION()
	void PressKey(float key);

private:
	TArray<float> recordedInput;
	bool playing = false;
	float remainingTime = 0;


};
