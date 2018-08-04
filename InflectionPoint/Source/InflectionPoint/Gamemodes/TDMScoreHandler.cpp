// Fill out your copyright notice in the Description page of Project Settings.

#include "InflectionPoint.h"
#include "Gameplay/CharacterInfoProvider.h"
#include "Gamemodes/TDMPlayerStateBase.h" 
#include "TDMScoreHandler.h"


// Sets default values for this component's properties
UTDMScoreHandler::UTDMScoreHandler() {DebugPrint(__FILE__, __LINE__);
}

float UTDMScoreHandler::GetKillerScoreChange(AController * KilledPlayer, AController* KillingPlayer) {DebugPrint(__FILE__, __LINE__);
	UCharacterInfoProvider* killerInfo = KillingPlayer ? KillingPlayer->GetCharacter()->FindComponentByClass<UCharacterInfoProvider>() : NULL;
	UCharacterInfoProvider* killedInfo = KilledPlayer->GetCharacter()->FindComponentByClass<UCharacterInfoProvider>();

	if(!killerInfo)
		return 0;

	if(IsTeamKill(killedInfo, killerInfo)) {DebugPrint(__FILE__, __LINE__);
		return ScorePointsForTeamKill;
	} else {DebugPrint(__FILE__, __LINE__);
		if(!killedInfo->IsAReplay())
			return ScorePointsForPlayerKill;

		if(killedInfo->IsAReplay())
			return ScorePointsForReplayKill;
	}
	return 0;
}

float UTDMScoreHandler::GetKilledScoreChange(AController * KilledPlayer, AController* KillingPlayer) {DebugPrint(__FILE__, __LINE__);
	return ScorePointsForDeath;
}


void UTDMScoreHandler::AddKill(AController * KilledPlayer, AController* KillingPlayer) {DebugPrint(__FILE__, __LINE__);
	UpdatePlayerScoreAfterKill(KilledPlayer, KillingPlayer);
	UpdateKillDeathNumbers(KilledPlayer, KillingPlayer);
}

void UTDMScoreHandler::UpdatePlayerScoreAfterKill(AController * KilledPlayer, AController* KillingPlayer) {DebugPrint(__FILE__, __LINE__);
	UCharacterInfoProvider* killerInfo = KillingPlayer ? KillingPlayer->GetCharacter()->FindComponentByClass<UCharacterInfoProvider>() : NULL;
	UCharacterInfoProvider* killedInfo = KilledPlayer->GetCharacter()->FindComponentByClass<UCharacterInfoProvider>();

	ATDMPlayerStateBase* killedState = Cast<ATDMPlayerStateBase>(killedInfo->PlayerState);
	killedState->Score += GetKilledScoreChange(KilledPlayer, KillingPlayer);

	if(!killerInfo)
		return;
	ATDMPlayerStateBase* killerState = Cast<ATDMPlayerStateBase>(killerInfo->PlayerState);
	killerState->Score += GetKillerScoreChange(KilledPlayer, KillingPlayer);
}


void UTDMScoreHandler::UpdateKillDeathNumbers(AController * KilledPlayer, AController* KillingPlayer) {DebugPrint(__FILE__, __LINE__);
	UCharacterInfoProvider* killerInfo = KillingPlayer ? KillingPlayer->GetCharacter()->FindComponentByClass<UCharacterInfoProvider>() : NULL;
	UCharacterInfoProvider* killedInfo = KilledPlayer->GetCharacter()->FindComponentByClass<UCharacterInfoProvider>();

	ATDMPlayerStateBase* killedState = Cast<ATDMPlayerStateBase>(killedInfo->PlayerState);
	killedState->Deaths++;
	if(!killerInfo)
		return;

	ATDMPlayerStateBase* killerState = Cast<ATDMPlayerStateBase>(killerInfo->PlayerState);

	if(IsTeamKill(killedInfo, killerInfo)) {DebugPrint(__FILE__, __LINE__);
		killerState->TeamKills++;
	} else {DebugPrint(__FILE__, __LINE__);
		if(!killedInfo->IsAReplay())
			killerState->PlayerKills++;

		if(killedInfo->IsAReplay())
			killerState->ReplayKills++;
	}
}

bool UTDMScoreHandler::IsTeamKill(UCharacterInfoProvider* killedInfo, UCharacterInfoProvider* killerInfo) {DebugPrint(__FILE__, __LINE__);
	return Cast<ATDMPlayerStateBase>(killedInfo->PlayerState)->Team == Cast<ATDMPlayerStateBase>(killerInfo->PlayerState)->Team;
}

void UTDMScoreHandler::UpdateScoresForNextRound() {DebugPrint(__FILE__, __LINE__);
	for(int i = 0; i < GetGameState()->PlayerArray.Num(); i++) {DebugPrint(__FILE__, __LINE__);
		auto tdmPlayerState = Cast<ATDMPlayerStateBase>(GetGameState()->PlayerArray[i]);
		tdmPlayerState->AddScoreToTotalScore();
		tdmPlayerState->ResetScore();
	}
}

void UTDMScoreHandler::SetCurrentScoresToTotalScore() {DebugPrint(__FILE__, __LINE__);
	for(int i = 0; i < GetGameState()->PlayerArray.Num(); i++) {DebugPrint(__FILE__, __LINE__);
		auto tdmPlayerState = Cast<ATDMPlayerStateBase>(GetGameState()->PlayerArray[i]);
		tdmPlayerState->SetCurrentScoreToTotalScore();
	}
}

void UTDMScoreHandler::SelectWinnerTeamForRound() {DebugPrint(__FILE__, __LINE__);
	int bestScore = -1;
	int bestTeam = 0;
	for(int team = 1; team <= GetGameState()->TeamCount; team++) {DebugPrint(__FILE__, __LINE__);
		int score = GetGameState()->GetTeamScore(team);
		if(score > bestScore) {DebugPrint(__FILE__, __LINE__);
			bestScore = score;
			bestTeam = team;
		}
	}
	GetGameState()->TeamWins[bestTeam] ++;
}

ATDMGameStateBase* UTDMScoreHandler::GetGameState() {DebugPrint(__FILE__, __LINE__);
	auto gameState = GetWorld()->GetGameState<ATDMGameStateBase>();
	AssertNotNull(gameState, GetWorld(), __FILE__, __LINE__);
	return gameState;
}

int UTDMScoreHandler::GetWinningTeam() {DebugPrint(__FILE__, __LINE__);
	int winningTeam = 1;
	int maxWins = 0;
	for(int team = 1; team <= GetGameState()->TeamCount; team++) {DebugPrint(__FILE__, __LINE__);
		int wins = GetGameState()->TeamWins[team];
		if(wins > maxWins) {DebugPrint(__FILE__, __LINE__);
			maxWins = wins;
			winningTeam = team;
		}
	}
	return winningTeam;
}

int UTDMScoreHandler::GetLosingTeam() {DebugPrint(__FILE__, __LINE__);
	int losingTeam = 1;
	int minWins = 1000;
	for(int team = 1; team <= GetGameState()->TeamCount; team++) {DebugPrint(__FILE__, __LINE__);
		int wins = GetGameState()->TeamWins[team];
		if(wins <= minWins) {DebugPrint(__FILE__, __LINE__);
			minWins = wins;
			losingTeam = team;
		}
	}
	return losingTeam;
}