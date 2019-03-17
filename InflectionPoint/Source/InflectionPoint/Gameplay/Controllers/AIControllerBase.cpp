// Fill out your copyright notice in the Description page of Project Settings.

#include "InflectionPoint.h"
#include "Gameplay/CharacterInfoProvider.h"
#include "Gameplay/Characters/ReplayCharacterBase.h"
#include "AIControllerBase.h"


AAIControllerBase::AAIControllerBase(const FObjectInitializer& ObjectInitializer) :
	Super(ObjectInitializer) {
}

void AAIControllerBase::Initialize(APlayerController* OwningController) {
	AssertNotNull(OwningController, GetWorld(), __FILE__, __LINE__);
	InitializeOwner(OwningController, OwningController->PlayerState);
}

void AAIControllerBase::InitializeOwner(APlayerController* OwningController, APlayerState* OwnerPlayerState) {
	AssertNotNull(OwningController, GetWorld(), __FILE__, __LINE__);
	AssertNotNull(OwnerPlayerState, GetWorld(), __FILE__, __LINE__);
	AssertNotNull(GetCharacter(), GetWorld(), __FILE__, __LINE__);

	OwningPlayerController = OwningController;
	UCharacterInfoProvider* provider = GetCharacter()->FindComponentByClass<UCharacterInfoProvider>();
	AssertNotNull(provider, GetWorld(), __FILE__, __LINE__);
	provider->PlayerState = OwnerPlayerState;
}

