// Fill out your copyright notice in the Description page of Project Settings.

#include "InflectionPoint.h"
#include "InputRecorder.h"
#include "Utils/CheckFunctions.h"

// Sets default values for this component's properties
UInputRecorder::UInputRecorder() {
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;
}


// Called when the game starts
void UInputRecorder::BeginPlay() {
	Super::BeginPlay();

	owner = (ABaseCharacter*)GetOwner();
}

void UInputRecorder::InitializeBindings(UInputComponent * inputComponent) {
	inputComponent->BindAction("Jump", IE_Pressed, this, &UInputRecorder::RecordStartJump);
	inputComponent->BindAction("Jump", IE_Released, this, &UInputRecorder::RecordStopJump);

	inputComponent->BindAction("Fire", IE_Pressed, this, &UInputRecorder::RecordOnFire);
	inputComponent->BindAction("DEBUG_Fire", IE_Pressed, this, &UInputRecorder::RecordOnDebugFire);

	inputComponent->BindAxis("MoveForward", this, &UInputRecorder::RecordMoveForward);
	inputComponent->BindAxis("MoveRight", this, &UInputRecorder::RecordMoveRight);
}

void UInputRecorder::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction * ThisTickFunction) {
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	passedTime += DeltaTime;
}

void UInputRecorder::RecordKeyPressed(FKey key) {
	FRotator rotCapsule = owner->GetCapsuleComponent()->GetComponentRotation();
	FRotator rotCamera = owner->GetFirstPersonCameraComponent()->GetComponentRotation();
	ServerRecordKeyPressed(key, passedTime, rotCapsule.Yaw, rotCamera.Pitch);
}

bool UInputRecorder::ServerRecordKeyPressed_Validate(FKey key, float time, float capsuleYaw, float cameraPitch) {
	return true;
}

void UInputRecorder::ServerRecordKeyPressed_Implementation(FKey key, float time, float capsuleYaw, float cameraPitch) {
	TTuple<float, float, float> tt(time, capsuleYaw, cameraPitch);
	Inputs.Add(TPair<FKey, TTuple<float, float, float>>(TPairInitializer<FKey, TTuple<float, float, float>>(key, tt)));
}

void UInputRecorder::RecordMoveForward(float val) {
	if(val != 0) {
		ServerRecordMoveForward(val, passedTime);
	}
}

bool UInputRecorder::ServerRecordMoveForward_Validate(float value, float time) {
	return true;
}

void UInputRecorder::ServerRecordMoveForward_Implementation(float Value, float time) {
	if(Value != 0) {
		MovementsForward.Add(time);
		MovementsForward.Add(Value);
	}
}

void UInputRecorder::RecordMoveRight(float val) {
	if(val != 0) {
		ServerRecordMoveRight(val, passedTime);
	}
}

bool UInputRecorder::ServerRecordMoveRight_Validate(float value, float time) {
	return true;
}

void UInputRecorder::ServerRecordMoveRight_Implementation(float Value, float time) {
	if(Value != 0) {
		MovementsRight.Add(time);
		MovementsRight.Add(Value);
	}
}

void UInputRecorder::RecordStartJump() {
	RecordKeyPressed(EKeys::SpaceBar);
}

void UInputRecorder::RecordStopJump() {

}

void UInputRecorder::RecordOnFire() {
	RecordKeyPressed(EKeys::LeftMouseButton);
}

void UInputRecorder::RecordOnDebugFire() {
	RecordKeyPressed(EKeys::RightMouseButton);
}