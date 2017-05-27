// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Components/ActorComponent.h"
#include "PositionRecorder.generated.h"


UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class INFLECTIONPOINT_API UPositionRecorder : public UActorComponent {
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	UPositionRecorder();

	// Called when the game starts
	virtual void BeginPlay() override;

	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	UFUNCTION(Server, Unreliable, WithValidation)
		void ServerRecordPosition(float time, FVector pos);

	void StartRecording();

	void StopRecording();

	bool IsRecording();

	UPROPERTY(EditAnywhere, Category = General)
		bool BeginRecordOnBeginPlay = false;

	UPROPERTY(EditAnywhere, Category = General)
		float RecordInterval = 0.1f;

	UPROPERTY(EditAnywhere, Category = Debug)
		bool CreateDebugPoints = false;

	UPROPERTY(EditAnywhere, Category = Debug)
		FColor DebugColor;

	TArray<TPair<float, FVector>> RecordArray;

private:
	float passedTime = 0.f;
	bool isRecording = false;
	float startRecordTimeSeconds;
	float lastRecordTimeSeconds;
};
