// Fill out your copyright notice in the Description page of Project Settings.

#include "InflectionPoint.h"
#include "Gameplay/Characters/BaseCharacter.h"
#include "RotationReplayer.h"
#include "Utils/CheckFunctions.h"
#include "Utils/TimerFunctions.h"

// Sets default values for this component's properties
URotationReplayer::URotationReplayer() {}


// Called when the game starts
void URotationReplayer::BeginPlay() {
	Super::BeginPlay();

	AssertTrue(GetOwner()->IsA(ABaseCharacter::StaticClass()), GetWorld(), __FILE__, __LINE__);
	owner = (ABaseCharacter*)GetOwner();
}

void URotationReplayer::StartReplay(TArray<float> yaws, TArray<float> pitches) {
	while(yaws.Num() > 0) {
		float yaw = yaws.Last();
		yaws.RemoveAt(yaws.Num() - 1);
		float wait = yaws.Last();
		yaws.RemoveAt(yaws.Num() - 1);

		StartTimer(this, GetWorld(), "ApplyYaw", wait, yaw);
	}

	while(pitches.Num() > 0) {
		float pitch = pitches.Last();
		pitches.RemoveAt(pitches.Num() - 1);
		float wait = pitches.Last();
		pitches.RemoveAt(pitches.Num() - 1);

		FTimerHandle TimerHandle;
		FTimerDelegate TimerDel;
		TimerDel.BindUFunction(this, FName("ApplyPitch"), pitch);

		GetWorld()->GetTimerManager().SetTimer(TimerHandle, TimerDel, wait, false, wait);
	}
}

void URotationReplayer::ApplyYaw(float value) {
	FRotator rot = owner->GetCapsuleComponent()->GetComponentRotation();
	rot.Yaw = value;
	rot.Roll = 0;
	rot.Pitch = 0;
	owner->GetCapsuleComponent()->SetWorldRotation(rot);

	FRotator rot2 = owner->GetFirstPersonCameraComponent()->GetComponentRotation();
	rot2.Yaw = value;
	owner->GetFirstPersonCameraComponent()->SetWorldRotation(rot2);
}

void URotationReplayer::ApplyPitch(float value) {
	FRotator rot = owner->GetFirstPersonCameraComponent()->GetComponentRotation();
	rot.Pitch = value;
	rot.Roll = 0;
	owner->GetFirstPersonCameraComponent()->SetWorldRotation(rot);
}
