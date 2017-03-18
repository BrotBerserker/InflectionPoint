// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Components/ActorComponent.h"
#include "Gameplay/Characters/BaseCharacter.h"
#include "RotationRecorder.generated.h"



UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class INFLECTIONPOINT_API URotationRecorder : public UActorComponent {
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	URotationRecorder();

	// Called when the game starts
	virtual void BeginPlay() override;

	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	void StartRecording();

	void StopRecording();

	bool IsRecording();

	UPROPERTY(EditAnywhere, Category = General)
		bool BeginRecordOnBeginPlay = false;

	UPROPERTY(EditAnywhere, Category = General)
		float RecordInterval = 0.1f;

	TArray<float> Yaws;
	TArray<float> Pitches;

	ABaseCharacter * owner;

private:
	bool isRecording = false;
	float startRecordTimeSeconds;
	float lastRecordTimeSeconds;

};
