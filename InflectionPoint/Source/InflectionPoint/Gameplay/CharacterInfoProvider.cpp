// Fill out your copyright notice in the Description page of Project Settings.

#include "InflectionPoint.h"
#include "Gamemodes/TDMPlayerStateBase.h"
#include "CharacterInfoProvider.h"


// Sets default values for this component's properties
UCharacterInfoProvider::UCharacterInfoProvider() {
	PrimaryComponentTick.bCanEverTick = true;
}

FCharacterInfo UCharacterInfoProvider::GetCharacterInfo() {
	FString extraInfo = "";
	ATDMPlayerStateBase* tdmPlayerState = Cast<ATDMPlayerStateBase>(PlayerState);
	return FCharacterInfo(tdmPlayerState->PlayerName, tdmPlayerState->Team, IsReplay);
}