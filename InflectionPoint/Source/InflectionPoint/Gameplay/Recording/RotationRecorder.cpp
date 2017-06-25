// Fill out your copyright notice in the Description page of Project Settings.

#include "InflectionPoint.h"
#include "RotationRecorder.h"

// ----------------------
// | (!) NOT USED!  (!) |
// ----------------------

// Sets default values for this component's properties
URotationRecorder::URotationRecorder() {
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;
}


// Called when the game starts
void URotationRecorder::BeginPlay() {
	Super::BeginPlay();

	owner = (ABaseCharacter*)GetOwner();

	if(BeginRecordOnBeginPlay) {
		StartRecording();
	}

}


// Called every frame
void URotationRecorder::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) {
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	passedTime += DeltaTime;

	if(!isRecording) {
		return;
	}

	if(passedTime - lastRecordTimeSeconds > RecordInterval) {
		// needs to perform record
		float time = passedTime - startRecordTimeSeconds;

		FRotator rotCapsule = owner->GetCapsuleComponent()->GetComponentRotation();
		FRotator rotCamera = owner->GetFirstPersonCameraComponent()->GetComponentRotation();
		ServerRecordRotation(time, rotCapsule.Yaw, rotCamera.Pitch);

		lastRecordTimeSeconds = passedTime;
	}
}

bool URotationRecorder::ServerRecordRotation_Validate(float time, float capsuleYaw, float cameraPitch) {
	return true;
}

void URotationRecorder::ServerRecordRotation_Implementation(float time, float capsuleYaw, float cameraPitch) {
	Yaws.Add(time);
	Yaws.Add(capsuleYaw);
	Pitches.Add(time);
	Pitches.Add(cameraPitch);
}

void URotationRecorder::StopRecording() {
	isRecording = false;
}

void URotationRecorder::StartRecording() {
	isRecording = true;
	startRecordTimeSeconds = GetWorld()->GetTimeSeconds();
	lastRecordTimeSeconds = -RecordInterval;
}

bool URotationRecorder::IsRecording() {
	return isRecording;
}

