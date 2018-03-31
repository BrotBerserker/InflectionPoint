// Fill out your copyright notice in the Description page of Project Settings.

#include "InflectionPoint.h"
#include "Gameplay/CharacterInfoProvider.h"
#include "Gameplay/Characters/ReplayCharacterBase.h"
#include "AIControllerBase.h"


AAIControllerBase::AAIControllerBase(const FObjectInitializer& ObjectInitializer) :
	Super(ObjectInitializer) {
}

void AAIControllerBase::Initialize(APlayerController* OwningController) {
	OwningPlayerController = OwningController;

	AssertNotNull(OwningPlayerController, GetWorld(), __FILE__, __LINE__);
	AssertNotNull(OwningPlayerController->PlayerState, GetWorld(), __FILE__, __LINE__);
	AssertNotNull(GetCharacter(), GetWorld(), __FILE__, __LINE__);
	UCharacterInfoProvider* provider = GetCharacter()->FindComponentByClass<UCharacterInfoProvider>();
	AssertNotNull(provider, GetWorld(), __FILE__, __LINE__);
	provider->PlayerState = OwningPlayerController->PlayerState;
}
