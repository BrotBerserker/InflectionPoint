// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Components/ActorComponent.h"
#include "Gameplay/Characters/BaseCharacter.h"
#include "InputRecorder.generated.h"



UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class INFLECTIONPOINT_API UInputRecorder : public UActorComponent {
	GENERATED_BODY()

public:

	FDateTime start;

	// Sets default values for this component's properties
	UInputRecorder();

	// Called when the game starts
	virtual void BeginPlay() override;

	virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction * ThisTickFunction) override;

	void InitializeBindings(UInputComponent * inputComponent);

	TMap<FString, TArray<TTuple<float, float, float>>> KeysPressed;
	TMap<FString, TArray<TTuple<float, float, float>>> KeysReleased;


private:
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

	UFUNCTION(Unreliable, Server, WithValidation)
		void ServerRecordKeyPressed(const FString& key, float time, float capsuleYaw, float cameraPitch);

	void RecordKeyReleased(FString key);

	UFUNCTION(Unreliable, Server, WithValidation)
		void ServerRecordKeyReleased(const FString& key, float time, float capsuleYaw, float cameraPitch);

	ABaseCharacter * owner;
	float passedTime = 0.f;

};
