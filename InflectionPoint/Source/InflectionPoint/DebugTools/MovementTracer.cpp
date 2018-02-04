// Fill out your copyright notice in the Description page of Project Settings.

#include "InflectionPoint.h"
#include "MovementTracer.h"
#include "Gameplay/Characters/BaseCharacter.h"
#include "DebugTools/InflectionPointCheatManager.h"
#include "DrawDebugHelpers.h"


// Sets default values for this component's properties
UMovementTracer::UMovementTracer() {
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

}


// Called when the game starts
void UMovementTracer::BeginPlay() {
	Super::BeginPlay();

}


// Called every frame
void UMovementTracer::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) {
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	auto cheatManager = Cast<UInflectionPointCheatManager>(GetWorld()->GetFirstPlayerController()->CheatManager);
	if(!(Enabled || (cheatManager && cheatManager->IsLineTracingEnabled))) {
		return;
	}

	DrawDebugPoint(GetWorld(), GetOwner()->GetTransform().GetLocation(), 10, LineColor, true);
	FVector directionVector = GetOwner()->GetTransform().GetLocation() + GetOwner()->GetActorForwardVector() * 15;
	DrawDebugDirectionalArrow(GetWorld(), GetOwner()->GetTransform().GetLocation(), directionVector, 15, LineColor, true, -1, 0, 0.5f);
	ABaseCharacter* bc = (ABaseCharacter*)GetOwner();
	FRotator cameraRot = bc->FirstPersonCameraComponent->GetComponentRotation();
	FVector cameraDirectionVector = cameraRot.Vector() * 15 + GetOwner()->GetTransform().GetLocation(); //cameraRot.RotateVector(directionVector);

	DrawDebugDirectionalArrow(GetWorld(), GetOwner()->GetTransform().GetLocation(), cameraDirectionVector, 15, LineColor, true, -1, 0, 0.5f);

}

