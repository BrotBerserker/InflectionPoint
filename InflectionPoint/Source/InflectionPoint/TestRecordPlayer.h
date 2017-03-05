// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Components/ActorComponent.h"
#include "PositionRecorder.h"
#include "TestRecordPlayer.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class INFLECTIONPOINT_API UTestRecordPlayer : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UTestRecordPlayer();

	// Called when the game starts
	virtual void BeginPlay() override;
	
	// Called every frame
	virtual void TickComponent( float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction ) override;


	void PLayReplay();


	UPROPERTY(EditAnywhere)
		UInputComponent* InputComponent;

	UPROPERTY(EditAnywhere)
		UPositionRecorder* PositionRecorder;

	UFUNCTION()
		void PerformMovingStep(FTimeStamp current, FTimeStamp next);	
};
