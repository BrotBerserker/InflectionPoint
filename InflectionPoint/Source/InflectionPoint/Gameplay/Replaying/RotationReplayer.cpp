// Fill out your copyright notice in the Description page of Project Settings.

#include "InflectionPoint.h"
#include "Gameplay/Characters/BaseCharacter.h"
#include "RotationReplayer.h"


// Sets default values for this component's properties
URotationReplayer::URotationReplayer() {
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;
}


// Called when the game starts
void URotationReplayer::BeginPlay() {
	Super::BeginPlay();

	owner = (ABaseCharacter*)GetOwner();
}


// Called every frame
void URotationReplayer::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) {
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

}

void URotationReplayer::StartReplay(TArray<float> yaws, TArray<float> pitches) {
	while(yaws.Num() > 0) {
		float yaw = yaws.Last();
		yaws.RemoveAt(yaws.Num() - 1);
		float wait = yaws.Last();
		yaws.RemoveAt(yaws.Num() - 1);

		//UE_LOG(LogTemp, Warning, TEXT("Waiting %f and then applying yaw: %f!"), wait, yaw);

		FTimerHandle TimerHandle;
		FTimerDelegate TimerDel;
		TimerDel.BindUFunction(this, FName("ApplyYaw"), yaw);

		GetWorld()->GetTimerManager().SetTimer(TimerHandle, TimerDel, wait, false, wait);
	}

	while(pitches.Num() > 0) {
		float pitch = pitches.Last();
		pitches.RemoveAt(pitches.Num() - 1);
		float wait = pitches.Last();
		pitches.RemoveAt(pitches.Num() - 1);

		//UE_LOG(LogTemp, Warning, TEXT("Waiting %f and then applying pitch: %f!"), wait, pitch);

		FTimerHandle TimerHandle;
		FTimerDelegate TimerDel;
		TimerDel.BindUFunction(this, FName("ApplyPitch"), pitch);

		GetWorld()->GetTimerManager().SetTimer(TimerHandle, TimerDel, wait, false, wait);
	}
}

void URotationReplayer::ApplyYaw(float value) {
	//UE_LOG(LogTemp, Warning, TEXT("Applying yaw: %f!"), value);
	//AddControllerYawInput(value);
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
	//AddControllerPitchInput(value);
	//AddActorLocalRotation(FQuat(0, 0, value, 0));
	FRotator rot = owner->GetFirstPersonCameraComponent()->GetComponentRotation();
	rot.Pitch = value;
	rot.Roll = 0;
	//rot.Yaw = 0;
	owner->GetFirstPersonCameraComponent()->SetWorldRotation(rot);
}
