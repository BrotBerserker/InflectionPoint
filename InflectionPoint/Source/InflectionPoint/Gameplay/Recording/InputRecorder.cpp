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


	start = FDateTime::UtcNow();


	owner = (ABaseCharacter*)GetOwner();
}

void UInputRecorder::InitializeBindings(UInputComponent * inputComponent) {
	inputComponent->BindAction("Jump", IE_Pressed, this, &UInputRecorder::RecordStartJump);
	inputComponent->BindAction("Jump", IE_Released, this, &UInputRecorder::RecordStopJump);

	inputComponent->BindAction("Fire", IE_Pressed, this, &UInputRecorder::RecordStartFire);
	inputComponent->BindAction("Fire", IE_Released, this, &UInputRecorder::RecordStopFire);

	inputComponent->BindAction("DEBUG_Fire", IE_Pressed, this, &UInputRecorder::RecordStartDebugFire);
	inputComponent->BindAction("DEBUG_Fire", IE_Released, this, &UInputRecorder::RecordStopDebugFire);

	inputComponent->BindAxis("MoveForward", this, &UInputRecorder::RecordMoveForward);
	inputComponent->BindAxis("MoveRight", this, &UInputRecorder::RecordMoveRight);
}

void UInputRecorder::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction * ThisTickFunction) {
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	passedTime += DeltaTime;
}

void UInputRecorder::RecordKeyPressed(FString key) {
	FRotator rotCapsule = owner->GetCapsuleComponent()->GetComponentRotation();
	FRotator rotCamera = owner->GetFirstPersonCameraComponent()->GetComponentRotation();
	ServerRecordKeyPressed(key, passedTime, rotCapsule.Yaw, rotCamera.Pitch);
}

bool UInputRecorder::ServerRecordKeyPressed_Validate(const FString& key, float time, float capsuleYaw, float cameraPitch) {
	return true;
}

void UInputRecorder::ServerRecordKeyPressed_Implementation(const FString& key, float time, float capsuleYaw, float cameraPitch) {
	TTuple<float, float, float> tt(time, capsuleYaw, cameraPitch);
	if(!KeysPressed.Contains(key)) {
		KeysPressed.Add(key, TArray<TTuple<float, float, float>>());
	}
	KeysPressed[key].Add(tt);
}

void UInputRecorder::RecordKeyReleased(FString key) {
	FRotator rotCapsule = owner->GetCapsuleComponent()->GetComponentRotation();
	FRotator rotCamera = owner->GetFirstPersonCameraComponent()->GetComponentRotation();
	ServerRecordKeyReleased(key, passedTime, rotCapsule.Yaw, rotCamera.Pitch);
}

bool UInputRecorder::ServerRecordKeyReleased_Validate(const FString& key, float time, float capsuleYaw, float cameraPitch) {
	return true;
}

void UInputRecorder::ServerRecordKeyReleased_Implementation(const FString& key, float time, float capsuleYaw, float cameraPitch) {
	TTuple<float, float, float> tt(time, capsuleYaw, cameraPitch);
	if(!KeysReleased.Contains(key)) {
		KeysReleased.Add(key, TArray<TTuple<float, float, float>>());
	}
	KeysReleased[key].Add(tt);
}

void UInputRecorder::RecordMoveForward(float val) {
	if(movingForward == val) {
		return;
	}

	if(val > 0) {
		RecordKeyPressed("MoveForward");
	} else if(val < 0) {
		RecordKeyPressed("MoveBackward");
	} else {
		RecordKeyReleased(movingForward > 0 ? "MoveForward" : "MoveBackward");
	}
	movingForward = val;
}

void UInputRecorder::RecordMoveRight(float val) {
	if(movingRight == val) {
		return;
	}

	if(val > 0) {
		RecordKeyPressed("MoveRight");
	} else if(val < 0) {
		RecordKeyPressed("MoveLeft");
	} else {
		RecordKeyReleased(movingRight > 0 ? "MoveRight" : "MoveLeft");
	}
	movingRight = val;
}

void UInputRecorder::RecordStartJump() {
	RecordKeyPressed("Jump");
}

void UInputRecorder::RecordStopJump() {
	RecordKeyReleased("Jump");
}

void UInputRecorder::RecordStartFire() {
	// WTF? TODO
	UE_LOG(LogTemp, Warning, TEXT("Shot recorded: %f"), passedTime);
	UE_LOG(LogTemp, Warning, TEXT("Shot recorded: %f"), (FDateTime::UtcNow() - start).GetMilliseconds());
	UE_LOG(LogTemp, Warning, TEXT("Shot recorded: %s"), *(FDateTime::UtcNow() - start).ToString());
	RecordKeyPressed("Fire");
}

void UInputRecorder::RecordStopFire() {
	RecordKeyReleased("Fire");
}

void UInputRecorder::RecordStartDebugFire() {
	RecordKeyPressed("DEBUG_Fire");
}

void UInputRecorder::RecordStopDebugFire() {
	RecordKeyReleased("DEBUG_Fire");
}