// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "BaseCharacter.h"
#include "Gameplay/Recording/PlayerStateRecorder.h"
#include "ReplayControlledFPSCharacter.generated.h"


/**
*
*/
UCLASS()
class INFLECTIONPOINT_API AReplayControlledFPSCharacter : public ABaseCharacter {
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	AReplayControlledFPSCharacter();

	virtual void BeginPlay() override;

	// Called every frame
	virtual void Tick(float DeltaTime) override;

	void StartReplay(TArray<FRecordedPlayerState> recordData);


	UFUNCTION()
		void PressKey(FString key);
	UFUNCTION()
		void ReleaseKey(FString key);

	UFUNCTION()
		void ApplyYaw(float value);

	UFUNCTION()
		void ApplyPitch(float value);

	UFUNCTION()
		void ReplayMoveForward(float value);

	UFUNCTION()
		void ReplayMoveRight(float value);


	TArray<FRecordedPlayerState> RecordData;

	bool IsReplaying = false;

	float passedTime = 0.f;
	int replayIndex = 0;
	int lastReplayIndex = 0;
	TArray<FString> pressedButtons;
	bool isForwardPressed = false;
	bool isBackwordPressed = false;
	bool isRightPressed = false;
	bool isLeftPressed = false;
private:
	void StartTimerForKeyChanged(TPair<FString, TArray<TTuple<float, float, float>>> & element, FString timerFunction);

};
