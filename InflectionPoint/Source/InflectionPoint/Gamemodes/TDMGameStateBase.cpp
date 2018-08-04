// Fill out your copyright notice in the Description page of Project Settings.

#include "InflectionPoint.h"
#include "Gamemodes/TDMPlayerStateBase.h"
#include "TDMGameStateBase.h"


void ATDMGameStateBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const {DebugPrint(__FILE__, __LINE__);
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ATDMGameStateBase, CurrentRound);
	DOREPLIFETIME(ATDMGameStateBase, MaxRoundNum);
	DOREPLIFETIME(ATDMGameStateBase, CurrentPhase);
	DOREPLIFETIME(ATDMGameStateBase, MaxPhaseNum);
	DOREPLIFETIME(ATDMGameStateBase, TeamWins);
	DOREPLIFETIME(ATDMGameStateBase, MaxPlayers);
	DOREPLIFETIME(ATDMGameStateBase, NumPlayers);
}

int ATDMGameStateBase::GetTeamScore(int team) {DebugPrint(__FILE__, __LINE__);
	double teamScore = 0;
	for(int i = 0; i < PlayerArray.Num(); i++) {DebugPrint(__FILE__, __LINE__);
		auto tdmPlayerState = Cast<ATDMPlayerStateBase>(PlayerArray[i]);
		AssertNotNull(tdmPlayerState, GetWorld(), __FILE__, __LINE__);
		if(tdmPlayerState && tdmPlayerState->Team == team)
			teamScore += tdmPlayerState->Score;
	}
	return teamScore;
}

void ATDMGameStateBase::ResetPlayerScores() {DebugPrint(__FILE__, __LINE__);
	for(int i = 0; i < PlayerArray.Num(); i++) {DebugPrint(__FILE__, __LINE__);
		auto tdmPlayerState = Cast<ATDMPlayerStateBase>(PlayerArray[i]);
		AssertNotNull(tdmPlayerState, GetWorld(), __FILE__, __LINE__);
		tdmPlayerState->ResetScore();
	}
}

void ATDMGameStateBase::ResetTotalPlayerScores() {DebugPrint(__FILE__, __LINE__);
	for(int i = 0; i < PlayerArray.Num(); i++) {DebugPrint(__FILE__, __LINE__);
		auto tdmPlayerState = Cast<ATDMPlayerStateBase>(PlayerArray[i]);
		AssertNotNull(tdmPlayerState, GetWorld(), __FILE__, __LINE__);
		tdmPlayerState->ResetTotalScore();
	}
}
