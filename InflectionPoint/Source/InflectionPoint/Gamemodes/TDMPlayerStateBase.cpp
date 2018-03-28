// Fill out your copyright notice in the Description page of Project Settings.

#include "InflectionPoint.h"
#include "TDMPlayerStateBase.h"

ATDMPlayerStateBase::ATDMPlayerStateBase() {
	bUseCustomPlayerNames = true;
}


void ATDMPlayerStateBase::GetLifetimeReplicatedProps(TArray< FLifetimeProperty > & OutLifetimeProps) const {
	DOREPLIFETIME(ATDMPlayerStateBase, Team);
	DOREPLIFETIME(ATDMPlayerStateBase, PlayerStartGroup);
	DOREPLIFETIME(ATDMPlayerStateBase, PlayerKills);
	DOREPLIFETIME(ATDMPlayerStateBase, ReplayKills);
	DOREPLIFETIME(ATDMPlayerStateBase, Deaths);
	DOREPLIFETIME(ATDMPlayerStateBase, TeamKills);
	DOREPLIFETIME(ATDMPlayerStateBase, Score);
	DOREPLIFETIME(ATDMPlayerStateBase, IsAlive);
	DOREPLIFETIME(ATDMPlayerStateBase, Ping);
	DOREPLIFETIME(ATDMPlayerStateBase, ReplicatedPlayerName);
}

void ATDMPlayerStateBase::ResetScore() {
	Deaths = 0;
	PlayerKills = 0;
	ReplayKills = 0;
	TeamKills = 0;
	Score = 0;
	IsAlive = 1;
}

void ATDMPlayerStateBase::SetPlayerName(const FString& S) {
	Super::SetPlayerName(S);
	ReplicatedPlayerName = S;
}

FString ATDMPlayerStateBase::GetPlayerNameCustom() const {
	return ReplicatedPlayerName;
}