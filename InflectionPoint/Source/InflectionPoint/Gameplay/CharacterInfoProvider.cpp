// Fill out your copyright notice in the Description page of Project Settings.

#include "InflectionPoint.h"
#include "Gamemodes/TDMPlayerStateBase.h"
#include "CharacterInfoProvider.h"


// Sets default values for this component's properties
UCharacterInfoProvider::UCharacterInfoProvider() {
	PrimaryComponentTick.bCanEverTick = true;
}

void UCharacterInfoProvider::GetLifetimeReplicatedProps(TArray< FLifetimeProperty > & OutLifetimeProps) const {
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UCharacterInfoProvider, PlayerState);
	DOREPLIFETIME(UCharacterInfoProvider, IsReplay);
}

FCharacterInfo UCharacterInfoProvider::GetCharacterInfo() {
	// When getting the character info too early (e.g. just after spawn), the playerstate might not be set
	if(PlayerState == NULL) {
		return FCharacterInfo();
	}
	ATDMPlayerStateBase* tdmPlayerState = Cast<ATDMPlayerStateBase>(PlayerState);
	return FCharacterInfo(tdmPlayerState->GetPlayerName(), tdmPlayerState->Team, IsReplay);
}