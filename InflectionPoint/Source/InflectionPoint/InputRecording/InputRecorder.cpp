// Fill out your copyright notice in the Description page of Project Settings.

#include "InflectionPoint.h"
#include "InputRecorder.h"


// Sets default values for this component's properties
UInputRecorder::UInputRecorder() {
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}


// Called when the game starts
void UInputRecorder::BeginPlay() {
	Super::BeginPlay();

	// Sch�n w�rs
	// GetOwner()->InputComponent->BindAction("InputRecording", IE_Pressed, this, &UInputRecorder::RecordInput);

	UInputComponent* inputComponent = GetOwner()->InputComponent;
	inputComponent->BindAction("Jump", IE_Pressed, this, &UInputRecorder::StartJump);
	inputComponent->BindAction("Jump", IE_Released, this, &UInputRecorder::StopJump);

	inputComponent->BindAction("Fire", IE_Pressed, this, &UInputRecorder::OnFire);

	inputComponent->BindAxis("MoveForward", this, &UInputRecorder::MoveForward);
	inputComponent->BindAxis("MoveRight", this, &UInputRecorder::MoveRight);

	inputComponent->BindAxis("Turn", this, &UInputRecorder::RecordYaw);
	//inputComponent->BindAxis("TurnRate", this, &UInputRecorder::TurnAtRate);
	inputComponent->BindAxis("LookUp", this, &UInputRecorder::RecordPitch);
	//inputComponent->BindAxis("LookUpRate", this, &UInputRecorder::LookUpAtRate);

}

void UInputRecorder::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction * ThisTickFunction) {
	passedTime += DeltaTime;
}

void UInputRecorder::RecordKeyPressed(Key key) {
	//UE_LOG(LogTemp, Warning, TEXT("Key pressed: %i after %f ms"), key + 0, passedTime);
	Inputs.Add(passedTime);
	Inputs.Add(key + 0);
	//UE_LOG(LogTemp, Warning, TEXT("%i Eintr�ge in der Liste!"), Inputs.Num());
}

void UInputRecorder::OnFire() {
	RecordKeyPressed(Key::LMB);
}

void UInputRecorder::MoveForward(float Value) {
	if(Value != 0) {
		MovementsForward.Add(passedTime);
		MovementsForward.Add(Value);
	}

}

void UInputRecorder::MoveRight(float Value) {
	if(Value != 0) {
		MovementsRight.Add(passedTime);
		MovementsRight.Add(Value);
	}
}

void UInputRecorder::StartJump() {
	RecordKeyPressed(Key::SPACE);
}

void UInputRecorder::StopJump() {

}

void UInputRecorder::RecordYaw(float Value) {
	if(Value != 0.f) {
		Yaws.Add(passedTime);
		Yaws.Add(Value);
	}
}

void UInputRecorder::RecordPitch(float Value) {
	if(Value != 0.f) {
		Pitches.Add(passedTime);
		Pitches.Add(Value);
	}
}



