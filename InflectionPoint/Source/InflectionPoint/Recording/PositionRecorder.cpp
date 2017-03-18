// Fill out your copyright notice in the Description page of Project Settings.

#include "InflectionPoint.h"
#include "PositionRecorder.h"
#include "DrawDebugHelpers.h"


// Sets default values for this component's properties
UPositionRecorder::UPositionRecorder() {
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}


// Called when the game starts
void UPositionRecorder::BeginPlay() {
	Super::BeginPlay();

	if(BeginRecordOnBeginPlay) {
		StartRecording();
	}
}


// Called every frame
void UPositionRecorder::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) {
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if(!isRecording) {
		return;
	}

	float currentTimeSeconds = GetWorld()->GetTimeSeconds();

	if(currentTimeSeconds - lastRecordTimeSeconds > RecordInterval) {
		// needs to perform record
		FVector pos = GetOwner()->GetTransform().GetLocation();
		float t = currentTimeSeconds - startRecordTimeSeconds;
		RecordArray.Add(TPair<float, FVector>(TPairInitializer<float, FVector>(t, pos)));
		lastRecordTimeSeconds = currentTimeSeconds;

		if(CreateDebugPoints) {
			DrawDebugPoint(GetWorld(), pos, 20, DebugColor, true);
		}
	}
}

void UPositionRecorder::StopRecording() {
	//UE_LOG(LogTemp, Warning, TEXT("stop recording"));

	isRecording = false;
	//lastRecordTimeSeconds = GetWorld()->GetTimeSeconds();

}

void UPositionRecorder::StartRecording() {
	//UE_LOG(LogTemp, Warning, TEXT("start recording"));
	isRecording = true;
	startRecordTimeSeconds = GetWorld()->GetTimeSeconds();
	lastRecordTimeSeconds = -RecordInterval;
	/*FTimeStamp stamp;
	stamp.TimeSeconds = startRecordTimeSeconds;
	stamp.Location = GetOwner()->GetTransform().GetLocation();
	stamp.Rotation = GetOwner()->GetTransform().GetRotation();
	recordArray.Add(stamp);*/
}

bool UPositionRecorder::IsRecording() {
	return isRecording;
}

