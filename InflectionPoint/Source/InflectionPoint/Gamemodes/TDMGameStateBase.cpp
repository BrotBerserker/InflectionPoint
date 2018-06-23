// Fill out your copyright notice in the Description page of Project Settings.

#include "InflectionPoint.h"
#include "Gamemodes/TDMPlayerStateBase.h"
#include "TDMGameStateBase.h"


void ATDMGameStateBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const {
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ATDMGameStateBase, CurrentRound);
	DOREPLIFETIME(ATDMGameStateBase, MaxRoundNum);
	DOREPLIFETIME(ATDMGameStateBase, CurrentPhase);
	DOREPLIFETIME(ATDMGameStateBase, TeamWins);
}

int ATDMGameStateBase::GetTeamScore(int team) {
	double teamScore = 0;
	for(auto playerState : PlayerArray) {
		auto tdmPlayerState = Cast<ATDMPlayerStateBase>(playerState);
		if(tdmPlayerState->Team == team)
			teamScore += tdmPlayerState->Score;
	}
	return teamScore;
}
