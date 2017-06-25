// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Components/ActorComponent.h"
#include "RotationReplayer.generated.h"

// ----------------------
// | (!) NOT USED!  (!) |
// ----------------------

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class INFLECTIONPOINT_API URotationReplayer : public UActorComponent {
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	URotationReplayer();

	// Called when the game starts
	virtual void BeginPlay() override;

	void StartReplay(TArray<float> yaws, TArray<float> pitches);

	UFUNCTION()
		void ApplyYaw(float value);

	UFUNCTION()
		void ApplyPitch(float value);

private:
	ABaseCharacter * owner;

};
