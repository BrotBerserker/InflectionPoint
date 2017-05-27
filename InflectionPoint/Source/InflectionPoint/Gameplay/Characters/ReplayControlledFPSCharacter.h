// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "BaseCharacter.h"
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
	virtual void Tick(float DeltaTime) override;

	void StartReplay(TMap<FString, TArray<TTuple<float, float, float>>> keysPressed, TMap<FString, TArray<TTuple<float, float, float>>> keysReleased);


	UFUNCTION()
		void PressKey(float yaw, float pitch, FString key);
	UFUNCTION()
		void ReleaseKey(float yaw, float pitch, FString key);

	UFUNCTION()
		void ApplyYaw(float value);

	UFUNCTION()
		void ApplyPitch(float value);

	UFUNCTION()
		void ReplayMoveForward(float value);

	UFUNCTION()
		void ReplayMoveRight(float value);


	TMap<FKey, TArray<TTuple<float, float, float>>> KeysPressed;
	TMap<FKey, TArray<TTuple<float, float, float>>> KeysReleased;

	bool isForwardPressed = false;
	bool isBackwordPressed = false;
	bool isRightPressed = false;
	bool isLeftPressed = false;
private:
	void StartTimerForKeyChanged(TPair<FString, TArray<TTuple<float, float, float>>> & element, FString timerFunction);
};
