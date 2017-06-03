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
	virtual void Tick(float deltaTime) override;

	UFUNCTION(BlueprintCallable, Category = "InflectionPoint|Replay")
		void StartReplay(TArray<FRecordedPlayerState> recordData);

	UFUNCTION(BlueprintCallable, Category = "InflectionPoint|Replay")
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
	
public:

	UPROPERTY(EditAnywhere, Category = General)
		float CorrectionRadius = 10.f;

	UPROPERTY(EditAnywhere, Category = General)
		float PositionCorrectionInterval = 0.1f;

	UPROPERTY(EditAnywhere, Category = Debug)
		bool CreateDebugCorrectionSpheres = true;

	UPROPERTY(EditAnywhere, Category = Debug)
		FColor DebugHitColor = FColorList::Yellow;

	UPROPERTY(EditAnywhere, Category = Debug)
		FColor DebugMissColor = FColorList::LightSteelBlue;

private:
	TArray<FRecordedPlayerState> RecordData;
	bool IsReplaying = false;
	float PassedTime = 0.f;
	float PassedTimeSinceLastCorrection = 0.f;
	int ReplayIndex = 0;
	TArray<FString> PressedKeys;

	void UpdatePressedKeys();

	void UpdatePressedKeys(FRecordedPlayerState &recordDataStep);

	void UpdateReleasedKeys(FRecordedPlayerState &recordDataStep);

	bool TryCorrectPosition(FVector correctPosition);

	bool IsAtProperPosition(FVector correctPosition);
};
