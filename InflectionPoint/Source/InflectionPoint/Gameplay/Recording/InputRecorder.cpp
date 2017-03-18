// Fill out your copyright notice in the Description page of Project Settings.

#include "InflectionPoint.h"
#include "InputRecorder.h"


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

	UInputComponent* inputComponent = GetOwner()->InputComponent;
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

	TTuple<float, float, float> tt(passedTime, rotCapsule.Yaw, rotCamera.Pitch);
	Inputs.Add(TPair<FKey,TTuple<float,float,float>>(TPairInitializer<FKey, TTuple<float, float, float>>(key, tt)));
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

void UInputRecorder::RecordMoveForward(float Value) {
	if(Value != 0) {
		MovementsForward.Add(passedTime);
		MovementsForward.Add(Value);
	}
}

void UInputRecorder::RecordMoveRight(float Value) {
	if(Value != 0) {
		MovementsRight.Add(passedTime);
		MovementsRight.Add(Value);
	}
}