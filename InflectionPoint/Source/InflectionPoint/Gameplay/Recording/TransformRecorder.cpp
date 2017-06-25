// Fill out your copyright notice in the Description page of Project Settings.

#include "InflectionPoint.h"
#include "TransformRecorder.h"
#include "Utils/TimerFunctions.h"

// ----------------------
// | (!) NOT USED!  (!) |
// ----------------------

// Sets default values for this component's properties
UTransformRecorder::UTransformRecorder() {
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;
}


// Called when the game starts
void UTransformRecorder::BeginPlay() {
	Super::BeginPlay();

	if(BeginRecordOnBeginPlay) {
		StartRecording();
	}
}

TArray<FTimeStamp> UTransformRecorder::StopRecording() {
	auto temp = TArray<FTimeStamp>(recordArray);
	recordArray.Reset();
	isRecording = false;
	lastRecordTimeSeconds = GetWorld()->GetTimeSeconds() - RecordInterval;
	return temp;
}

void UTransformRecorder::StartRecording() {
	isRecording = true;
	startRecordTimeSeconds = GetWorld()->GetTimeSeconds();
	lastRecordTimeSeconds = GetWorld()->GetTimeSeconds();
	FTimeStamp stamp;
	stamp.TimeSeconds = startRecordTimeSeconds;
	stamp.Location = GetOwner()->GetTransform().GetLocation();
	stamp.Rotation = GetOwner()->GetTransform().GetRotation();
	recordArray.Add(stamp);
}

bool UTransformRecorder::IsRecording() {
	return isRecording;
}


// Called every frame
void UTransformRecorder::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) {
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if(isRecording) {
		float currendTimeSeconds = GetWorld()->GetTimeSeconds();

		if(currendTimeSeconds - lastRecordTimeSeconds > RecordInterval) {
			// needs to perform record
			FTimeStamp stamp;
			stamp.Location = GetOwner()->GetTransform().GetLocation();
			stamp.Rotation = GetOwner()->GetTransform().GetRotation();


			bool locChanged = (recordArray.Last().Location - stamp.Location).Size() > MinLocationDistance;
			bool rotChanged = recordArray.Last().Rotation.AngularDistance(stamp.Rotation) > MinRotationDistance;
			if(locChanged || rotChanged) {
				stamp.TimeSeconds = currendTimeSeconds - startRecordTimeSeconds;
				recordArray.Add(stamp);
			}
			lastRecordTimeSeconds = currendTimeSeconds;
		}
	}
}