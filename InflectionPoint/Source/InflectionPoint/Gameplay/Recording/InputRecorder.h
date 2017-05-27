// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Components/ActorComponent.h"
#include "Gameplay/Characters/BaseCharacter.h"
#include "InputRecorder.generated.h"



UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class INFLECTIONPOINT_API UInputRecorder : public UActorComponent {
	GENERATED_BODY()

public:

	// Sets default values for this component's properties
	UInputRecorder();

	// Called when the game starts
	virtual void BeginPlay() override;

	virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction * ThisTickFunction) override;

	void InitializeBindings(UInputComponent * inputComponent);

	TArray<TPair<FKey, TTuple<float, float, float>>> Inputs;
	TArray<float> MovementsForward;
	TArray<float> MovementsRight;


private:
	void RecordStartJump();

	void RecordStopJump();

	void RecordOnFire();

	void RecordOnDebugFire();

	void RecordMoveForward(float val);

	void RecordMoveRight(float val);

	void RecordKeyPressed(FKey key);

	UFUNCTION(Reliable, Server, WithValidation)
		void ServerRecordMoveForward(float val, float time);

	UFUNCTION(Reliable, Server, WithValidation)
		void ServerRecordMoveRight(float val, float time);

	UFUNCTION(Reliable, Server, WithValidation)
		void ServerRecordKeyPressed(FKey key, float time, float capsuleYaw, float cameraPitch);

	ABaseCharacter * owner;
	float passedTime = 0.f;

};
