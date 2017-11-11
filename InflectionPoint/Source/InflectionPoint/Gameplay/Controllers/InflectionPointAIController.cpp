// Fill out your copyright notice in the Description page of Project Settings.

#include "InflectionPoint.h"
#include "Gameplay/CharacterInfoProvider.h"
#include "Gameplay/Characters/ReplayControlledFPSCharacter.h"
#include "InflectionPointAIController.h"


AInflectionPointAIController::AInflectionPointAIController(const FObjectInitializer& ObjectInitializer) :
	Super(ObjectInitializer) {

	CharacterInfoProvider = CreateDefaultSubobject<UCharacterInfoProvider>(TEXT("CharacterInfoProvider"));
}

void AInflectionPointAIController::Initialize(APlayerController* OwningController) {
	OwningPlayerController = OwningController;

	AssertNotNull(OwningPlayerController, GetWorld(), __FILE__, __LINE__);
	AssertNotNull(OwningPlayerController->PlayerState, GetWorld(), __FILE__, __LINE__);
	CharacterInfoProvider->PlayerState = OwningPlayerController->PlayerState;
	CharacterInfoProvider->IsReplay = true;
}
