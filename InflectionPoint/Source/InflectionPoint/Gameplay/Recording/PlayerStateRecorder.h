// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Components/ActorComponent.h"
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
		TArray<FString> ButtonsPressed;

	FRecordedPlayerState() {
		ButtonsPressed = TArray<FString>();
	}

	FRecordedPlayerState(float timestamp, FVector position, float capsuleYaw, float cameraPitch, TArray<FString> buttonsPressed) {
		Timestamp = timestamp;
		Position = position;
		CapsuleYaw = capsuleYaw;
		CameraPitch = cameraPitch;
		ButtonsPressed = buttonsPressed;
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



};
