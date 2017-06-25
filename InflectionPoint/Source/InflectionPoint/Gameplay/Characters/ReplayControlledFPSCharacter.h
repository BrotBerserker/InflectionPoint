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
	/* ------------- */
	/*   Functions   */
	/* ------------- */

	/** Construcotr, sets default values for this component's properties */
	AReplayControlledFPSCharacter();

	/** BeginPlay, empty */
	virtual void BeginPlay() override;

	/** Tick, responsible for replaying */
	virtual void Tick(float deltaTime) override;

	/** Starts the replay */
	UFUNCTION(BlueprintCallable, Category = "InflectionPoint|Replay")
		void StartReplay(TArray<FRecordedPlayerState> RecordData);

	/** Stops the replay */
	UFUNCTION(BlueprintCallable, Category = "InflectionPoint|Replay")
		void StopReplay();

	/** Act as if the player would have pressed the given key */
	UFUNCTION()
		void PressKey(FString key);

	/** Act as if the player would have released the given key */
	UFUNCTION()
		void ReleaseKey(FString key);

	/** Act as if the player was holding down the given key */
	UFUNCTION()
		void HoldKey(FString key);

	/** Updates the capsule's and the camera's yaw */
	UFUNCTION()
		void ApplyYaw(float value);

	/** Updates the camera's pitch */
	UFUNCTION()
		void ApplyPitch(float value);


public:
	/* ------------------ */
	/* Blueprint Settings */
	/* ------------------ */

	/** Max distance between the replay and the original position. If this distance is exceeded, the replay's position will not be corrected anymore. */
	UPROPERTY(EditAnywhere, Category = General)
		float CorrectionRadius = 10.f;

	/** Time to wait before two position corrections */
	UPROPERTY(EditAnywhere, Category = General)
		float PositionCorrectionInterval = 0.1f;

	/** If true, debug spheres will be created to show the CorrectionRadius and if the position has been corrected or not */
	UPROPERTY(EditAnywhere, Category = Debug)
		bool CreateDebugCorrectionSpheres = true;

	/** Sphere color if the position has been corrected */
	UPROPERTY(EditAnywhere, Category = Debug)
		FColor DebugHitColor = FColorList::Yellow;

	/** Sphere color if the position has not been corrected */
	UPROPERTY(EditAnywhere, Category = Debug)
		FColor DebugMissColor = FColorList::LightSteelBlue;

private:
	TArray<FRecordedPlayerState> recordData;
	bool isReplaying = false;
	float passedTime = 0.f;
	float passedTimeSinceLastCorrection = 0.f;
	int replayIndex = 0;
	TArray<FString> pressedKeys;

	void UpdatePressedKeys();

	void UpdatePressedKeys(FRecordedPlayerState &recordDataStep);

	void UpdateReleasedKeys(FRecordedPlayerState &recordDataStep);

	bool TryCorrectPosition(FVector correctPosition);

	bool IsAtProperPosition(FVector correctPosition);
};
