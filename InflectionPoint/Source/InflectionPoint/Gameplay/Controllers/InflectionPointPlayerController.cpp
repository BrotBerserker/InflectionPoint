// Fill out your copyright notice in the Description page of Project Settings.

#include "InflectionPoint.h"
#include "DebugTools/InflectionPointCheatManager.h"
#include "Gameplay/CharacterInfoProvider.h"
#include "InflectionPointPlayerController.h"

AInflectionPointPlayerController::AInflectionPointPlayerController(const FObjectInitializer& ObjectInitializer) :
	Super(ObjectInitializer) {
	CheatClass = UInflectionPointCheatManager::StaticClass();

	CharacterInfoProvider = CreateDefaultSubobject<UCharacterInfoProvider>(TEXT("CharacterInfoProvider"));
}

void AInflectionPointPlayerController::Possess(APawn* InPawn) {
	Super::Possess(InPawn);

	AssertNotNull(InPawn->PlayerState, GetWorld(), __FILE__, __LINE__);
	CharacterInfoProvider->PlayerState = InPawn->PlayerState;
}

void AInflectionPointPlayerController::ClientSetControlRotation_Implementation(FRotator rotation) {
	SetControlRotation(rotation);
}