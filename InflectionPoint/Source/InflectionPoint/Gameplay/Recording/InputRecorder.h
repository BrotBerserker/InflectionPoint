// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Components/ActorComponent.h"
#include "Gameplay/Characters/BaseCharacter.h"
#include "InputRecorder.generated.h"



UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class INFLECTIONPOINT_API UInputRecorder : public UActorComponent {
	GENERATED_BODY()

public:

	enum Key {
		LMB,
		RMB,
		SPACE
	};

	// Sets default values for this component's properties
	UInputRecorder();

	// Called when the game starts
	virtual void BeginPlay() override;

	virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction * ThisTickFunction) override;

	void StartJump();

	void StopJump();

	void OnFire();

	void OnDebugFire();

	void MoveForward(float Val);

	void MoveRight(float Val);

	void RecordYaw(float Rate);

	void RecordPitch(float Rate);

	void RecordKeyPressed(Key key);

	TArray<float> Inputs;
	TArray<float> Yaws;
	TArray<float> Pitches;
	TArray<float> MovementsForward;
	TArray<float> MovementsRight;

	ABaseCharacter * owner;

private:
	float passedTime = 0.f;



};
