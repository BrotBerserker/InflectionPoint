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

	AssertNotNull(InPawn->PlayerState, GetWorld(), __FILE__, __LINE__);
	CharacterInfoProvider->PlayerState = InPawn->PlayerState;
}

void APlayerControllerBase::ClientSetControlRotation_Implementation(FRotator rotation) {
	SetControlRotation(rotation);
}

void APlayerControllerBase::ClientRoundStarted_Implementation(int Round) {
	OnRoundStarted(Round);
}

void APlayerControllerBase::ClientShowKillInfo_Implementation(FCharacterInfo KillerInfo, float killerScoreChange, FCharacterInfo KilledInfo, float killedScoreChange, UTexture2D* WeaponImage) {
	OnKillInfoAdded(KillerInfo, killerScoreChange, KilledInfo, killedScoreChange, WeaponImage);
}

void APlayerControllerBase::ClientShowCountdownNumber_Implementation(int number) {
	OnCountdownUpdate(number);
}

void APlayerControllerBase::ClientSetIgnoreInput_Implementation(bool ignore) {
	ACharacter* character = GetCharacter();
	AssertNotNull(character, GetWorld(), __FILE__, __LINE__);

	if(ignore) {
		character->DisableInput(this);
	} else {
		character->EnableInput(this);
	}

}
