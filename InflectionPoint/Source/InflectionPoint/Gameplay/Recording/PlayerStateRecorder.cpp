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

	inputComponent->BindAction("Fire", IE_Pressed, this, &UPlayerStateRecorder::RecordStartFire);
	inputComponent->BindAction("Fire", IE_Released, this, &UPlayerStateRecorder::RecordStopFire);

	inputComponent->BindAction("DEBUG_Fire", IE_Pressed, this, &UPlayerStateRecorder::RecordStartDebugFire);
	inputComponent->BindAction("DEBUG_Fire", IE_Released, this, &UPlayerStateRecorder::RecordStopDebugFire);

	inputComponent->BindAxis("MoveForward", this, &UPlayerStateRecorder::RecordMoveForward);
	inputComponent->BindAxis("MoveRight", this, &UPlayerStateRecorder::RecordMoveRight);
}


// Called when the game starts
void UPlayerStateRecorder::BeginPlay() {
	Super::BeginPlay();

	passedTime = 0.f;

	owner = (ABaseCharacter*)GetOwner();
	AssertNotNull(owner, GetWorld(), __FILE__, __LINE__);
}


// Called every frame
void UPlayerStateRecorder::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) {
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	passedTime += DeltaTime;

	FVector pos = owner->GetTransform().GetLocation();
	float yaw = owner->GetCapsuleComponent()->GetComponentRotation().Yaw;
	float pitch = owner->GetFirstPersonCameraComponent()->GetComponentRotation().Pitch;

	RecordedPlayerStates.Add(FRecordedPlayerState(passedTime, pos, yaw, pitch, buttonsPressed));
}

void UPlayerStateRecorder::RecordKeyPressed(FString key) {
	UE_LOG(LogTemp, Warning, TEXT("[%s] pressed"), *(key));
	buttonsPressed.Add(key);
}

void UPlayerStateRecorder::RecordKeyReleased(FString key) {
	UE_LOG(LogTemp, Warning, TEXT("[%s] released"), *(key));
	buttonsPressed.Remove(key);
}

void UPlayerStateRecorder::RecordMoveForward(float val) {
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

void UPlayerStateRecorder::RecordMoveRight(float val) {
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

void UPlayerStateRecorder::RecordStartJump() {
	RecordKeyPressed("Jump");
}

void UPlayerStateRecorder::RecordStopJump() {
	RecordKeyReleased("Jump");
}

void UPlayerStateRecorder::RecordStartFire() {
	RecordKeyPressed("Fire");
}

void UPlayerStateRecorder::RecordStopFire() {
	RecordKeyReleased("Fire");
}

void UPlayerStateRecorder::RecordStartDebugFire() {
	RecordKeyPressed("DEBUG_Fire");
}

void UPlayerStateRecorder::RecordStopDebugFire() {
	RecordKeyReleased("DEBUG_Fire");
}

