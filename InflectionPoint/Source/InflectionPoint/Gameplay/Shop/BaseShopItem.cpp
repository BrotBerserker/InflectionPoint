// Fill out your copyright notice in the Description page of Project Settings.

#include "InflectionPoint.h"
#include "Gamemodes/TDMPlayerStateBase.h"
#include "Gameplay/Characters/BaseCharacter.h"
#include "BaseShopItem.h"

bool UBaseShopItem::IsAffordableForLocalPlayer() {
	if(!GetWorld()->GetFirstPlayerController())
		return false;
	auto playerState = Cast<ATDMPlayerStateBase>(GetWorld()->GetFirstPlayerController()->PlayerState);
	return IsAffordableForPlayer(playerState);
}



bool UBaseShopItem::IsAffordableForPlayer(ATDMPlayerStateBase* playerState) {
	if(!playerState)
		return false;
	return playerState->IPPoints >= IPPrice;
}