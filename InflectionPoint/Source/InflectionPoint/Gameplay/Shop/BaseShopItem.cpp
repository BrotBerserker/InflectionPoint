// Fill out your copyright notice in the Description page of Project Settings.

#include "InflectionPoint.h"
#include "Gamemodes/TDMPlayerStateBase.h"
#include "BaseShopItem.h"

bool UBaseShopItem::IsAffordableForLocalPlayer() {
	auto playerState = Cast<ATDMPlayerStateBase>(GetWorld()->GetFirstPlayerController()->PlayerState);
	return IsAffordableForPlayer(playerState);
}



bool UBaseShopItem::IsAffordableForPlayer(ATDMPlayerStateBase* playerState) {
	AssertNotNull(playerState, GetWorld(), __FILE__, __LINE__);
	return playerState->IPPoints >= IPPrice;
}