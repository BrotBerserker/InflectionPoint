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

	// Schön wärs
	// GetOwner()->InputComponent->BindAction("InputRecording", IE_Pressed, this, &UInputRecorder::RecordInput);

	UInputComponent* inputComponent = GetOwner()->InputComponent;
	inputComponent->BindAction("Jump", IE_Pressed, this, &UInputRecorder::StartJump);
	inputComponent->BindAction("Jump", IE_Released, this, &UInputRecorder::StopJump);

	inputComponent->BindAction("Fire", IE_Pressed, this, &UInputRecorder::OnFire);

	inputComponent->BindAxis("MoveForward", this, &UInputRecorder::MoveForward);
	inputComponent->BindAxis("MoveRight", this, &UInputRecorder::MoveRight);

	//	inputComponent->BindAxis("Turn", this, &APawn::AddControllerYawInput);
	inputComponent->BindAxis("TurnRate", this, &UInputRecorder::TurnAtRate);
	//	inputComponent->BindAxis("LookUp", this, &APawn::AddControllerPitchInput);
	inputComponent->BindAxis("LookUpRate", this, &UInputRecorder::LookUpAtRate);

}

void UInputRecorder::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction * ThisTickFunction) {
	passedTime += DeltaTime;
}

void UInputRecorder::RecordKeyPressed(Key key) {
	//UE_LOG(LogTemp, Warning, TEXT("Key pressed: %i after %f ms"), key + 0, passedTime);
	Inputs.Add(-passedTime);
	Inputs.Add(key + 0);
	//passedTime = 0.f;
}

void UInputRecorder::OnFire() {
	RecordKeyPressed(Key::LMB);
}

void UInputRecorder::MoveForward(float Value) {
	/*if(Value > 0) {
		RecordKeyPressed("W");
	}
	if(Value < 0) {
		RecordKeyPressed("S");
	}*/
}

void UInputRecorder::MoveRight(float Value) {
	/*if(Value > 0) {
		RecordKeyPressed("D");
	}
	if(Value < 0) {
		RecordKeyPressed("A");
	}*/
}

void UInputRecorder::StartJump() {
	RecordKeyPressed(Key::SPACE);
}

void UInputRecorder::StopJump() {

}

void UInputRecorder::TurnAtRate(float Rate) {

}

void UInputRecorder::LookUpAtRate(float Rate) {

}



