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
	void StopReplay();

	UFUNCTION()
		void PressKey(FString key);
	UFUNCTION()
		void ReleaseKey(FString key);
	UFUNCTION()
		void HoldKey(FString key);

	UFUNCTION()
		void ApplyYaw(float value);

	UFUNCTION()
		void ApplyPitch(float value);

	UFUNCTION()
		void ReplayMoveForward(float value);

	UFUNCTION()
		void ReplayMoveRight(float value);
	
private:
	TArray<FRecordedPlayerState> RecordData;

	bool IsReplaying = false;
	float PassedTime = 0.f;
	int ReplayIndex = 0;
	TArray<FString> PressedButtons;

	void UpdatePressedButtons();

	void UpdatePressedButtonsPressedKeys(FRecordedPlayerState &recordDataStep);

	void UpdatePressedButtonsReleasedKeys(FRecordedPlayerState &recordDataStep);
};
