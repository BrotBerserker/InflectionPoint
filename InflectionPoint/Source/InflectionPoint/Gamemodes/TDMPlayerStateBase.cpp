// Fill out your copyright notice in the Description page of Project Settings.

#include "InflectionPoint.h"
#include "TDMPlayerStateBase.h"




void ATDMPlayerStateBase::GetLifetimeReplicatedProps(TArray< FLifetimeProperty > & OutLifetimeProps) const {
	DOREPLIFETIME(ATDMPlayerStateBase, Team);
	DOREPLIFETIME(ATDMPlayerStateBase, PlayerStartGroup);
	DOREPLIFETIME(ATDMPlayerStateBase, PlayerName);
	DOREPLIFETIME(ATDMPlayerStateBase, PlayerKills);
	DOREPLIFETIME(ATDMPlayerStateBase, ReplayKills);
	DOREPLIFETIME(ATDMPlayerStateBase, Deaths);
	DOREPLIFETIME(ATDMPlayerStateBase, TeamKills);
	DOREPLIFETIME(ATDMPlayerStateBase, Score);
}

void ATDMPlayerStateBase::ResetScore() {
	Deaths = 0;
	PlayerKills = 0;
	ReplayKills = 0;
	TeamKills = 0;
	Score = 0;
}