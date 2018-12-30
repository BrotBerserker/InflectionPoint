// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "BaseCharacter.h"
#include "Gameplay/Recording/PlayerStateRecorder.h"
#include "ReplayCharacterBase.generated.h"


/**
*
*/
UCLASS()
class INFLECTIONPOINT_API AReplayCharacterBase : public ABaseCharacter {
	GENERATED_BODY()

		DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnFinishedReplaying);

public:
	/* -------------- */
	/*   Components   */
	/* -------------- */

	/** Mesh3P for post processing */
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = Mesh)
		class USkeletalMeshComponent* Mesh3PPostProcess;

public:
	/* ------------------ */
	/* Editor Settings */
	/* ------------------ */

	/** Max distance between the replay and the original position. If this distance is exceeded, the replay's position will not be corrected anymore. A negative value behaves like an "infinite" radius */
	UPROPERTY(EditAnywhere, Category = General)
		float CorrectionRadius = 10.f;

	/** Time to wait before two position corrections, a negative value will disable position correction */
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

	/** If the AI takes ofer the controll after replay is finished */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = General)
		bool AITakeoverAfterReplayEnd = true;

	/** Material to use when transforming the replay to an IP */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Materialize|Dematerialize")
		UMaterialInstance* DematerializeMaterial;

	/** Curve to use when dematerializing the replay */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Materialize|Dematerialize")
		UCurveFloat* DematerializeCurve;

	/** InflectionPoint class to spawn */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Materialize|Dematerialize")
		TSubclassOf<AActor> InflectionPoint;

	/** This material will not be set to DematerializeMaterial (prevent our "Transparent Material" from being overriden) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Materialize|Dematerialize")
		UMaterial* MaterialToIgnore;
public:
	/* ------------- */
	/*   Functions   */
	/* ------------- */

	/** Construcotr, sets default values for this component's properties */
	AReplayCharacterBase();

	/** BeginPlay, empty */
	virtual void BeginPlay() override;

	/** Tick, responsible for replaying */
	virtual void Tick(float deltaTime) override;

	bool IsReadyForInitialization() override;

	void Initialize() override;

	/** Starts the replay */
	UFUNCTION(BlueprintCallable, Category = "InflectionPoint|Replay")
		void StartReplay();

	/** Stops the replay */
	UFUNCTION(BlueprintCallable, Category = "InflectionPoint|Replay")
		void StopReplay();

	/** Sets the replay data */
	UFUNCTION(BlueprintCallable)
		void SetReplayData(TArray<FRecordedPlayerState> RecordData);

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

	UFUNCTION()
		bool HasFinishedReplaying();

	UFUNCTION()
		void DematerializeCallback(float value);

public:
	/* ------------- */
	/*    Events     */
	/* ------------- */
	UPROPERTY(BlueprintAssignable)
		FOnFinishedReplaying OnFinishedReplaying;

public:

	UPROPERTY(BlueprintReadWrite)
		bool CorrectPositions = true;

	// Index of the Replay (to tell the diference between the replays of a player)
	UPROPERTY(BlueprintReadWrite, Replicated)
		int ReplayIndex = -1;

private:

	TArray<FRecordedPlayerState> recordData;
	bool isReplaying = false;
	float passedTime = 0.f;
	float passedTimeSinceLastCorrection = 0.f;
	int replayIndex = 0;
	TArray<FString> pressedKeys;

	void UpdateKeys();

	void UpdateRotation();

	void UpdatePressedKeys(FRecordedPlayerState &recordDataStep);

	void UpdateReleasedKeys(FRecordedPlayerState &recordDataStep);

	void CorrectPosition(FVector correctPosition);

	bool CurrentPositionShouldBeCorrected();

	bool CurrentPositionIsInCorrectionRadius(float radius);

	void DrawDebugSphereAtCurrentPosition(bool positionHasBeenCorrected);

	UFUNCTION(NetMulticast, Reliable)
		void MulticastShowDematerializeAnimation();

	UFUNCTION(NetMulticast, Reliable)
		void MulticastUpdateCustomDepthStencil();


	void OverrideMaterials(USkeletalMeshComponent* Mesh, UMaterialInterface* Material);

	UFUNCTION()
		void TransformToInflectionPoint();

	UMaterialInstanceDynamic* dematerializeInstanceDynamic;
};
