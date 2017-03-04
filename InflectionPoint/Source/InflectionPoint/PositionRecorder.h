// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Components/ActorComponent.h"
#include "PositionRecorder.generated.h"

USTRUCT()
struct FTimeStamp {

	GENERATED_USTRUCT_BODY()

//	UPROPERTY()
	float TimeSeconds;
	
//	UPROPERTY()
	FVector Location;
	
//	UPROPERTY()
	FQuat Rotation;
};

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class INFLECTIONPOINT_API UPositionRecorder : public UActorComponent
{
	GENERATED_BODY()


public:	

	// Sets default values for this component's properties
	UPositionRecorder();

	// Called when the game starts
	virtual void BeginPlay() override;

	TArray<FTimeStamp> StopRecording();

	void StartRecording();

	bool IsRecording();
	
	// Called every frame
	virtual void TickComponent( float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction ) override;


	UPROPERTY(EditAnywhere, Category = General)
		bool BeginRecordOnBeginPlay = false;

	UPROPERTY(EditAnywhere, Category = General)
		float RecordInterval = 1.f;

	UPROPERTY(EditAnywhere, Category = General)
		float MinLocationDistance = 0.1f;

	UPROPERTY(EditAnywhere, Category = General)
		float MinRotationDistance = 0.1f;


private:
	TArray<FTimeStamp> recordArray;

	bool isRecording = false;
	float startRecordTimeSeconds;
	float lastRecordTimeSeconds;
};
