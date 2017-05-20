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

bool UInputRecorder::RecordKeyPressed_Validate(FKey key) {
	return true;
}

void UInputRecorder::RecordKeyPressed_Implementation(FKey key) {
	FRotator rotCapsule = owner->GetCapsuleComponent()->GetComponentRotation();
	FRotator rotCamera = owner->GetFirstPersonCameraComponent()->GetComponentRotation();

	TTuple<float, float, float> tt(passedTime, rotCapsule.Yaw, rotCamera.Pitch);
	Inputs.Add(TPair<FKey, TTuple<float, float, float>>(TPairInitializer<FKey, TTuple<float, float, float>>(key, tt)));
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

bool UInputRecorder::RecordMoveForward_Validate(float value) {
	return true;
}

void UInputRecorder::RecordMoveForward_Implementation(float Value) {
	if(Value != 0) {
		MovementsForward.Add(passedTime);
		MovementsForward.Add(Value);
	}
}

bool UInputRecorder::RecordMoveRight_Validate(float value) {
	return true;
}

void UInputRecorder::RecordMoveRight_Implementation(float Value) {
	if(Value != 0) {
		MovementsRight.Add(passedTime);
		MovementsRight.Add(Value);
	}
}