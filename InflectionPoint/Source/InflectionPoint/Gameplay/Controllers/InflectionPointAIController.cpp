// Fill out your copyright notice in the Description page of Project Settings.

#include "InflectionPoint.h"
#include "Gameplay/NameProvider.h"
#include "Gameplay/Characters/ReplayControlledFPSCharacter.h"
#include "InflectionPointAIController.h"


AInflectionPointAIController::AInflectionPointAIController(const FObjectInitializer& ObjectInitializer) :
	Super(ObjectInitializer) {

	NameProvider = CreateDefaultSubobject<UNameProvider>(TEXT("NameProvider"));
}

void AInflectionPointAIController::Initialize() {
	AssertNotNull(GetCharacter(), GetWorld(), __FILE__, __LINE__);

	AReplayControlledFPSCharacter* replayCharacter = Cast<AReplayControlledFPSCharacter>(GetCharacter());
	AssertNotNull(replayCharacter, GetWorld(), __FILE__, __LINE__);
	AssertNotNull(replayCharacter->OwningPlayerController, GetWorld(), __FILE__, __LINE__);
	AssertNotNull(replayCharacter->OwningPlayerController->PlayerState, GetWorld(), __FILE__, __LINE__);
	NameProvider->Name = Cast<AReplayControlledFPSCharacter>(GetCharacter())->OwningPlayerController->PlayerState->PlayerName + " (Replay)";
}
