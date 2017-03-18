// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Components/ActorComponent.h"
#include "Gameplay/Recording/TransformRecorder.h"
#include "TransformReplayer.generated.h"


UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class INFLECTIONPOINT_API UTransformReplayer : public UActorComponent {
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	UTransformReplayer();

	// Called when the game starts
	virtual void BeginPlay() override;

	void PLayReplay();


	UPROPERTY(EditAnywhere)
		UInputComponent* InputComponent;

	UPROPERTY(EditAnywhere)
		UTransformRecorder* PositionRecorder;

	UFUNCTION()
		void PerformMovingStep(FTimeStamp current, FTimeStamp next);
};
