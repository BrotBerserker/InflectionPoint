// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Components/ActorComponent.h"
#include "Gameplay/Weapons/WeaponInventory.h"
#include "PlayerStateRecorder.generated.h"

/** Represents a player's state (location, rotation, pressed keys) at a certain point of time */
USTRUCT(BlueprintType)
struct FRecordedPlayerState {
	GENERATED_BODY()

		UPROPERTY(BlueprintReadWrite)
		float Timestamp;

	UPROPERTY(BlueprintReadWrite)
		FVector Position;

	UPROPERTY(BlueprintReadWrite)
		float CapsuleYaw;

	UPROPERTY(BlueprintReadWrite)
		float CameraPitch;

	UPROPERTY(BlueprintReadWrite)
		TArray<FString> PressedKeys;

	UPROPERTY(BlueprintReadWrite)
		TArray<FString> ReleasedKeys;

	FRecordedPlayerState() {
		PressedKeys = TArray<FString>();
	}

	FRecordedPlayerState(float timestamp, FVector position, float capsuleYaw, float cameraPitch, TArray<FString> pressedKeys, TArray<FString> releasedKeys) {
		Timestamp = timestamp;
		Position = position;
		CapsuleYaw = capsuleYaw;
		CameraPitch = cameraPitch;
		PressedKeys = pressedKeys;
		ReleasedKeys = releasedKeys;
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
	UFUNCTION(Server, Reliable, WithValidation, BlueprintCallable)
		void ServerStartRecording();

	/** Resets the list of recorded PlayerStates */
	UFUNCTION(Unreliable, Server, WithValidation)
		void ServerResetRecordedPlayerStates();

public:
	/* -------------- */
	/*   Properties   */
	/* -------------- */

	/** List of recorded player states */
	UPROPERTY(BlueprintReadOnly)
		TArray<FRecordedPlayerState> RecordedPlayerStates;

public:
	UPROPERTY(BlueprintReadOnly)
		float passedTime;

private:
	bool recording = false;

	class ABaseCharacter* owner;
	TArray<FString> pressedKeys;
	TArray<FString> releasedKeys;

	TArray<FRecordedPlayerState> recordedPlayerStateQueue;

	/** Number of entries that have to be in the queue before they will be sent to the server */
	int MaxQueueEntries = 7;

	int movingForward = 0;

	int movingRight = 0;

	UFUNCTION(BlueprintCallable)
		void RecordKey(FString key, EInputEvent eventType);

	template<EInputEvent eventType>
	void RecordJump();

	template<EInputEvent eventType>
	void RecordToggleCrouching();

	template<EInputEvent eventType>
	void RecordAim();

	template<EInputEvent eventType>
	void RecordSprint();

	template<EInputEvent eventType>
	void RecordReload();

	template<EInputEvent eventType>
	void RecordEquipNextWeapon();

	template<EInputEvent eventType>
	void RecordEquipPreviousWeapon();

	UFUNCTION(BlueprintCallable)
		void RecordMoveForward(float val);

	void RecordMoveRight(float val);

	template<EInventorySlotPosition slot, EInputEvent eventType>
	void RecordEquipSpecificSlot();

public:
	UFUNCTION(Server, Reliable, WithValidation)
		void ServerRecordKeyPressed(const FString &key);

	UFUNCTION(Server, Reliable, WithValidation)
		void ServerRecordKeyReleased(const FString &key);

};
