// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Components/ActorComponent.h"
#include "Gameplay/Characters/BaseCharacter.h"
#include "PlayerStateRecorder.generated.h"

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
	// Sets default values for this component's properties
	UPlayerStateRecorder();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	void InitializeBindings(UInputComponent * inputComponent);

	void StartRecording();

public:
	TArray<FRecordedPlayerState> RecordedPlayerStates;

	int MaxQueueEntries = 7;

private:
	bool recording = false;
	float passedTime;
	ABaseCharacter* owner;
	TArray<FString> pressedKeys;

	TArray<FRecordedPlayerState> recordedPlayerStateQueue;

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

	UFUNCTION(Unreliable, Server, WithValidation)
		void ServerRecordPlayerState(float timestamp, FVector position, float capsuleYaw, float cameraPitch, const TArray<FString>& pressedKeys);

};
