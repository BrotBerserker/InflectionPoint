// Fill out your copyright notice in the Description page of Project Settings.

#include "InflectionPoint.h"
#include "RotationRecorder.h"


// Sets default values for this component's properties
URotationRecorder::URotationRecorder() {
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
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
		float t = passedTime - startRecordTimeSeconds;

		FRotator rotCapsule = owner->GetCapsuleComponent()->GetComponentRotation();
		Yaws.Add(t);
		Yaws.Add(rotCapsule.Yaw);

		FRotator rotCamera = owner->GetFirstPersonCameraComponent()->GetComponentRotation();
		Pitches.Add(t);
		Pitches.Add(rotCamera.Pitch);

		lastRecordTimeSeconds = passedTime;
	}
}

void URotationRecorder::StopRecording() {
	//UE_LOG(LogTemp, Warning, TEXT("stop recording"));

	isRecording = false;
	//lastRecordTimeSeconds = GetWorld()->GetTimeSeconds();

}

void URotationRecorder::StartRecording() {
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

bool URotationRecorder::IsRecording() {
	return isRecording;
}

