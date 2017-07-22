// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Components/ActorComponent.h"
#include "Gameplay/Characters/BaseCharacter.h"
#include "PlayerStateRecorder.generated.h"

/** Represents a player's state (location, rotation, pressed keys) at a certain point of time */
USTRUCT()
struct FRecordedPlayerState {
	GENERATED_BODY()

		UPROPERTY()
		float Timestamp;

	UPROPERTY()
		FVector Position;

	UPROPERTY()
		float CapsuleYaw;

	UPROPERTY()
		float CameraPitch;

	UPROPERTY()
		TArray<FString> PressedKeys;

	FRecordedPlayerState() {
		PressedKeys = TArray<FString>();
	}

	FRecordedPlayerState(float timestamp, FVector position, float capsuleYaw, float cameraPitch, TArray<FString> pressedKeys) {
		Timestamp = timestamp;
		Position = position;
		CapsuleYaw = capsuleYaw;
		CameraPitch = cameraPitch;
		PressedKeys = pressedKeys;
	}

	FString ToString() {
		FString keys = "[";
		for(auto key : PressedKeys) {
			keys += ", ";
			keys += key;
		}
		keys += "]";
		const FString stateString = FString::Printf(TEXT("PlayerState [Timestamp: %f, Position: %s, CapsuleYaw: %f, CameraPitch: %f, PressedKeys: %s]"), Timestamp, *Position.ToString(), CapsuleYaw, CameraPitch, *keys);
		return stateString;
	}

};

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class INFLECTIONPOINT_API UPlayerStateRecorder : public UActorComponent {
	GENERATED_BODY()

public:
	/* ------------- */
	/*   Functions   */
	/* ------------- */

	/** Constructor */
	UPlayerStateRecorder();

	/** BeginPlay, initializes variables */
	virtual void BeginPlay() override;

	/** Tick, responsible for recording */
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	/** Binds recording functions to key events */
	void InitializeBindings(UInputComponent * inputComponent);

	/** Starts recording */
	void StartRecording();

	/** Creates a RecordedPlayerState from the given parameters and adds it to the list of recorded player states */
	UFUNCTION(Unreliable, Server, WithValidation)
		void ServerRecordPlayerState(float Timestamp, FVector Position, float CapsuleYaw, float CameraPitch, const TArray<FString>& PressedKeys);

public:
	/* -------------- */
	/*   Properties   */
	/* -------------- */

	/** List of recorded player states */
	TArray<FRecordedPlayerState> RecordedPlayerStates;

private:
	bool recording = false;
	float passedTime;
	ABaseCharacter* owner;
	TArray<FString> pressedKeys;

	TArray<FRecordedPlayerState> recordedPlayerStateQueue;

	/** Number of entries that have to be in the queue before they will be sent to the server */
	int MaxQueueEntries = 7;

	int movingForward = 0;

	int movingRight = 0;

	void RecordStartJump();

	void RecordStopJump();

	void RecordStartFire();

	void RecordStopFire();

	void RecordStartDebugFire();

	void RecordStopDebugFire();

	void RecordMoveForward(float val);

	void RecordMoveRight(float val);

	void RecordKeyPressed(FString key);

	void RecordKeyReleased(FString key);

};
