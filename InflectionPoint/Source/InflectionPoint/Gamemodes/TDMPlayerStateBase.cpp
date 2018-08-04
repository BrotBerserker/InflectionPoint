// Fill out your copyright notice in the Description page of Project Settings.

#include "InflectionPoint.h"
#include "TDMPlayerStateBase.h"

ATDMPlayerStateBase::ATDMPlayerStateBase() {DebugPrint(__FILE__, __LINE__);
	bUseCustomPlayerNames = true;
}


void ATDMPlayerStateBase::GetLifetimeReplicatedProps(TArray< FLifetimeProperty > & OutLifetimeProps) const {DebugPrint(__FILE__, __LINE__);
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
}

void ATDMPlayerStateBase::AddScoreToTotalScore() {DebugPrint(__FILE__, __LINE__);
	TotalDeaths += Deaths;
	TotalPlayerKills += PlayerKills;
	TotalReplayKills += ReplayKills;
	TotalTeamKills += TeamKills;
	TotalScore += Score;
}

void ATDMPlayerStateBase::SetCurrentScoreToTotalScore() {DebugPrint(__FILE__, __LINE__);
	Deaths = TotalDeaths;
	PlayerKills = TotalPlayerKills;
	ReplayKills = TotalReplayKills;
	TeamKills = TotalTeamKills;
	Score = TotalScore;
}

void ATDMPlayerStateBase::ResetScore() {DebugPrint(__FILE__, __LINE__);
	Deaths = 0;
	PlayerKills = 0;
	ReplayKills = 0;
	TeamKills = 0;
	Score = 0;
	IsAlive = 1;
}

void ATDMPlayerStateBase::ResetTotalScore() {DebugPrint(__FILE__, __LINE__);
	TotalDeaths = 0;
	TotalPlayerKills = 0;
	TotalReplayKills = 0;
	TotalTeamKills = 0;
	TotalScore = 0;
}

void ATDMPlayerStateBase::SetPlayerName(const FString& S) {DebugPrint(__FILE__, __LINE__);
	Super::SetPlayerName(S);
	ReplicatedPlayerName = S;
}

FString ATDMPlayerStateBase::GetPlayerNameCustom() const {DebugPrint(__FILE__, __LINE__);
	return ReplicatedPlayerName;
}