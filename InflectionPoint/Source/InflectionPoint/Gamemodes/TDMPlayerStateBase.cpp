// Fill out your copyright notice in the Description page of Project Settings.

#include "InflectionPoint.h"
#include "Gameplay/Shop/BaseShopItem.h"
#include "TDMPlayerStateBase.h"

ATDMPlayerStateBase::ATDMPlayerStateBase() {
	bUseCustomPlayerNames = true;
}


void ATDMPlayerStateBase::GetLifetimeReplicatedProps(TArray< FLifetimeProperty > & OutLifetimeProps) const {
	DOREPLIFETIME(ATDMPlayerStateBase, Team);
	DOREPLIFETIME(ATDMPlayerStateBase, PlayerStartGroup);
	DOREPLIFETIME(ATDMPlayerStateBase, PlayerKills);
	DOREPLIFETIME(ATDMPlayerStateBase, TotalPlayerKills);
	DOREPLIFETIME(ATDMPlayerStateBase, ReplayKills);
	DOREPLIFETIME(ATDMPlayerStateBase, TotalReplayKills);
	DOREPLIFETIME(ATDMPlayerStateBase, Deaths);
	DOREPLIFETIME(ATDMPlayerStateBase, TotalDeaths);
	DOREPLIFETIME(ATDMPlayerStateBase, TeamKills);
	DOREPLIFETIME(ATDMPlayerStateBase, TotalTeamKills);
	DOREPLIFETIME(ATDMPlayerStateBase, Score);
	DOREPLIFETIME(ATDMPlayerStateBase, TotalScore);
	DOREPLIFETIME(ATDMPlayerStateBase, IsAlive);
	DOREPLIFETIME(ATDMPlayerStateBase, Ping);
	DOREPLIFETIME(ATDMPlayerStateBase, ReplicatedPlayerName);
	DOREPLIFETIME(ATDMPlayerStateBase, IPPoints);
	DOREPLIFETIME(ATDMPlayerStateBase, PurchasedShopItems);
}

void ATDMPlayerStateBase::AddScoreToTotalScore() {
	TotalDeaths += Deaths;
	TotalPlayerKills += PlayerKills;
	TotalReplayKills += ReplayKills;
	TotalTeamKills += TeamKills;
	TotalScore += Score;
}

void ATDMPlayerStateBase::SetCurrentScoreToTotalScore() {
	Deaths = TotalDeaths;
	PlayerKills = TotalPlayerKills;
	ReplayKills = TotalReplayKills;
	TeamKills = TotalTeamKills;
	Score = TotalScore;
}

void ATDMPlayerStateBase::ResetScore() {
	Deaths = 0;
	PlayerKills = 0;
	ReplayKills = 0;
	TeamKills = 0;
	Score = 0;
	IsAlive = 1;
}

void ATDMPlayerStateBase::ResetTotalScore() {
	TotalDeaths = 0;
	TotalPlayerKills = 0;
	TotalReplayKills = 0;
	TotalTeamKills = 0;
	TotalScore = 0;
}

void ATDMPlayerStateBase::SetPlayerName(const FString& S) {
	Super::SetPlayerName(S);
	ReplicatedPlayerName = S;
}

FString ATDMPlayerStateBase::GetPlayerNameCustom() const {
	return ReplicatedPlayerName;
}

bool ATDMPlayerStateBase::ServerPurchaseShopItem_Validate(TSubclassOf<class UBaseShopItem> itemClass) {
	return true;
}

void ATDMPlayerStateBase::ServerPurchaseShopItem_Implementation(TSubclassOf<class UBaseShopItem> itemClass) {
	auto item = itemClass.GetDefaultObject();
	AssertNotNull(item, GetWorld(), __FILE__, __LINE__);
	if(!item->IsAffordableForPlayer(this))
		return;
	IPPoints -= item->IPPrice;
	PurchasedShopItems.Add(item->GetClass());
}