// Fill out your copyright notice in the Description page of Project Settings.

#include "InflectionPoint.h"
#include "DebugTools/InflectionPointCheatManager.h"
#include "Gameplay/CharacterInfoProvider.h"
#include "Gameplay/Damage/MortalityProvider.h"
#include "PlayerControllerBase.h"

APlayerControllerBase::APlayerControllerBase(const FObjectInitializer& ObjectInitializer) :
	Super(ObjectInitializer) {
	CheatClass = UInflectionPointCheatManager::StaticClass();

	CharacterInfoProvider = CreateDefaultSubobject<UCharacterInfoProvider>(TEXT("CharacterInfoProvider"));
}

void APlayerControllerBase::BeginPlay() {
	Super::BeginPlay();
}

void APlayerControllerBase::Possess(APawn* InPawn) {
	Super::Possess(InPawn);

	SetInputMode(FInputModeGameOnly());

	AssertNotNull(InPawn->PlayerState, GetWorld(), __FILE__, __LINE__);
	CharacterInfoProvider->PlayerState = InPawn->PlayerState;

	UMortalityProvider* mortalityProvider = InPawn->FindComponentByClass<UMortalityProvider>();
	AssertNotNull(mortalityProvider, GetWorld(), __FILE__, __LINE__);
	
}

void APlayerControllerBase::ClientSetControlRotation_Implementation(FRotator rotation) {
	SetControlRotation(rotation);
}