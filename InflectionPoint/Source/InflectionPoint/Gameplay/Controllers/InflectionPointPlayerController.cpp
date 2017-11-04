// Fill out your copyright notice in the Description page of Project Settings.

#include "InflectionPoint.h"
#include "DebugTools/InflectionPointCheatManager.h"
#include "Gameplay/NameProvider.h"
#include "InflectionPointPlayerController.h"

AInflectionPointPlayerController::AInflectionPointPlayerController(const FObjectInitializer& ObjectInitializer) :
	Super(ObjectInitializer) {
	CheatClass = UInflectionPointCheatManager::StaticClass();

	NameProvider = CreateDefaultSubobject<UNameProvider>(TEXT("NameProvider"));
}

void AInflectionPointPlayerController::Possess(APawn* InPawn) {
	Super::Possess(InPawn);

	AssertNotNull(InPawn->PlayerState, GetWorld(), __FILE__, __LINE__);
	NameProvider->Name = InPawn->PlayerState->PlayerName;
}

void AInflectionPointPlayerController::ClientSetControlRotation_Implementation(FRotator rotation) {
	SetControlRotation(rotation);
}