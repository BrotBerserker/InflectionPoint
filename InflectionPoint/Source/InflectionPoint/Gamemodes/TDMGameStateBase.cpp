// Fill out your copyright notice in the Description page of Project Settings.

#include "InflectionPoint.h"
#include "Gamemodes/TDMPlayerStateBase.h"
#include "TDMGameStateBase.h"


void ATDMGameStateBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const {
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ATDMGameStateBase, CurrentRound);
	DOREPLIFETIME(ATDMGameStateBase, MaxRoundNum);
	DOREPLIFETIME(ATDMGameStateBase, CurrentPhase);
	DOREPLIFETIME(ATDMGameStateBase, MaxPhaseNum);
	DOREPLIFETIME(ATDMGameStateBase, TeamWins);
	DOREPLIFETIME(ATDMGameStateBase, MaxPlayers);
	DOREPLIFETIME(ATDMGameStateBase, NumPlayers);
}

int ATDMGameStateBase::GetTeamScore(int team) {
	double teamScore = 0;
	for(int i = 0; i < PlayerArray.Num(); i++) {
		auto tdmPlayerState = Cast<ATDMPlayerStateBase>(PlayerArray[i]);
		if(tdmPlayerState && tdmPlayerState->Team == team)
			teamScore += tdmPlayerState->Score;
	}
	return teamScore;
}

void ATDMGameStateBase::ResetPlayerScores() {
	for(int i = 0; i < PlayerArray.Num(); i++) {
		auto tdmPlayerState = Cast<ATDMPlayerStateBase>(PlayerArray[i]);
		tdmPlayerState->ResetScore();
	}
}