// Fill out your copyright notice in the Description page of Project Settings.

#include "InflectionPoint.h"
#include "PlayerStateRecorder.h"


// Sets default values for this component's properties
UPlayerStateRecorder::UPlayerStateRecorder() {
	PrimaryComponentTick.bCanEverTick = true;
}

void UPlayerStateRecorder::InitializeBindings(UInputComponent * inputComponent) {
	inputComponent->BindAction("Jump", IE_Pressed, this, &UPlayerStateRecorder::RecordStartJump);
	inputComponent->BindAction("Jump", IE_Released, this, &UPlayerStateRecorder::RecordStopJump);

	inputComponent->BindAxis("MoveForward", this, &UPlayerStateRecorder::RecordMoveForward);
	inputComponent->BindAxis("MoveRight", this, &UPlayerStateRecorder::RecordMoveRight);
}


// Called when the game starts
void UPlayerStateRecorder::BeginPlay() {
	Super::BeginPlay();

	//passedTime = 0.f;

	owner = (ABaseCharacter*)GetOwner();
	AssertNotNull(owner, GetWorld(), __FILE__, __LINE__);
}


// Called every frame
void UPlayerStateRecorder::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) {
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if(!recording) {
		return;
	}

	passedTime += DeltaTime;

	// Get data to be recorded
	FVector pos = owner->GetTransform().GetLocation();
	float yaw = owner->GetCapsuleComponent()->GetComponentRotation().Yaw;
	float pitch = owner->FirstPersonCameraComponent->GetComponentRotation().Pitch;

	RecordedPlayerStates.Add(FRecordedPlayerState(passedTime, pos, yaw, pitch, pressedKeys));
}

bool UPlayerStateRecorder::ServerStartRecording_Validate() {
	return true;
}

void UPlayerStateRecorder::ServerStartRecording_Implementation() {
	ServerResetRecordedPlayerStates();
	recordedPlayerStateQueue.Empty();
	passedTime = 0.f;
	recording = true;
}

bool UPlayerStateRecorder::ServerResetRecordedPlayerStates_Validate() {
	return true;
}

void UPlayerStateRecorder::ServerResetRecordedPlayerStates_Implementation() {
	RecordedPlayerStates.Empty();
}

bool UPlayerStateRecorder::ServerRecordKeyPressed_Validate(const FString &key) {
	return true;
}

void UPlayerStateRecorder::ServerRecordKeyPressed_Implementation(const FString &key) {
	RecordKeyPressed(key);
}

void UPlayerStateRecorder::RecordKeyPressed(const FString &key) {
	pressedKeys.Add(key);
}

bool UPlayerStateRecorder::ServerRecordKeyReleased_Validate(const FString &key) {
	return true;
}

void UPlayerStateRecorder::ServerRecordKeyReleased_Implementation(const FString &key) {
	RecordKeyReleased(key);
}

void UPlayerStateRecorder::RecordKeyReleased(const FString &key) {
	pressedKeys.Remove(key);
}

void UPlayerStateRecorder::RecordMoveForward(float val) {
	if(movingForward == val) {
		return;
	}

	if(val > 0) {
		ServerRecordKeyPressed("MoveForward");
	} else if(val < 0) {
		ServerRecordKeyPressed("MoveBackward");
	}

	if(movingForward > 0) {
		ServerRecordKeyReleased("MoveForward");
	} else if(movingForward < 0) {
		ServerRecordKeyReleased("MoveBackward");
	}

	movingForward = val;
}

void UPlayerStateRecorder::RecordMoveRight(float val) {
	if(movingRight == val) {
		return;
	}

	if(val > 0) {
		ServerRecordKeyPressed("MoveRight");
	} else if(val < 0) {
		ServerRecordKeyPressed("MoveLeft");
	}

	if(movingRight > 0) {
		ServerRecordKeyReleased("MoveRight");
	} else if(movingRight < 0) {
		ServerRecordKeyReleased("MoveLeft");
	}

	movingRight = val;
}

void UPlayerStateRecorder::RecordStartJump() {
	ServerRecordKeyPressed("Jump");
}

void UPlayerStateRecorder::RecordStopJump() {
	ServerRecordKeyReleased("Jump");
}
