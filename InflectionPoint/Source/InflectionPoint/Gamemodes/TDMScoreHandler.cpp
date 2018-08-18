// Fill out your copyright notice in the Description page of Project Settings.

#include "InflectionPoint.h"
#include "Gameplay/CharacterInfoProvider.h"
#include "Gamemodes/TDMPlayerStateBase.h" 
#include "TDMScoreHandler.h"


// Sets default values for this component's properties
UTDMScoreHandler::UTDMScoreHandler() {
}

float UTDMScoreHandler::GetKillerScoreChange(AController * KilledPlayer, AController* KillingPlayer) {
	UCharacterInfoProvider* killerInfo = KillingPlayer ? KillingPlayer->GetCharacter()->FindComponentByClass<UCharacterInfoProvider>() : NULL;
	UCharacterInfoProvider* killedInfo = KilledPlayer->GetCharacter()->FindComponentByClass<UCharacterInfoProvider>();

	if(!killerInfo)
		return 0;

	if(IsTeamKill(killedInfo, killerInfo)) {
		return ScorePointsForTeamKill;
	} else {
		if(!killedInfo->IsAReplay())
			return ScorePointsForPlayerKill;

		if(killedInfo->IsAReplay())
			return ScorePointsForReplayKill;
	}
	return 0;
}

float UTDMScoreHandler::GetKilledScoreChange(AController * KilledPlayer, AController* KillingPlayer) {
	return ScorePointsForDeath;
}


void UTDMScoreHandler::AddKill(AController * KilledPlayer, AController* KillingPlayer) {
	UpdatePlayerScoreAfterKill(KilledPlayer, KillingPlayer);
	UpdateKillDeathNumbers(KilledPlayer, KillingPlayer);
}

void UTDMScoreHandler::UpdatePlayerScoreAfterKill(AController * KilledPlayer, AController* KillingPlayer) {
	UCharacterInfoProvider* killerInfo = KillingPlayer ? KillingPlayer->GetCharacter()->FindComponentByClass<UCharacterInfoProvider>() : NULL;
	UCharacterInfoProvider* killedInfo = KilledPlayer->GetCharacter()->FindComponentByClass<UCharacterInfoProvider>();

	ATDMPlayerStateBase* killedState = Cast<ATDMPlayerStateBase>(killedInfo->PlayerState);
	killedState->Score += GetKilledScoreChange(KilledPlayer, KillingPlayer);

	if(!killerInfo)
		return;
	ATDMPlayerStateBase* killerState = Cast<ATDMPlayerStateBase>(killerInfo->PlayerState);
	killerState->Score += GetKillerScoreChange(KilledPlayer, KillingPlayer);
}


void UTDMScoreHandler::UpdateKillDeathNumbers(AController * KilledPlayer, AController* KillingPlayer) {
	UCharacterInfoProvider* killerInfo = KillingPlayer ? KillingPlayer->GetCharacter()->FindComponentByClass<UCharacterInfoProvider>() : NULL;
	UCharacterInfoProvider* killedInfo = KilledPlayer->GetCharacter()->FindComponentByClass<UCharacterInfoProvider>();

	ATDMPlayerStateBase* killedState = Cast<ATDMPlayerStateBase>(killedInfo->PlayerState);
	killedState->Deaths++;
	if(!killerInfo)
		return;

	ATDMPlayerStateBase* killerState = Cast<ATDMPlayerStateBase>(killerInfo->PlayerState);

	if(IsTeamKill(killedInfo, killerInfo)) {
		killerState->TeamKills++;
	} else {
		if(!killedInfo->IsAReplay())
			killerState->PlayerKills++;

		if(killedInfo->IsAReplay())
			killerState->ReplayKills++;
	}
}

bool UTDMScoreHandler::IsTeamKill(UCharacterInfoProvider* killedInfo, UCharacterInfoProvider* killerInfo) {
	return Cast<ATDMPlayerStateBase>(killedInfo->PlayerState)->Team == Cast<ATDMPlayerStateBase>(killerInfo->PlayerState)->Team;
}

void UTDMScoreHandler::UpdateScoresForNextRound() {
	for(int i = 0; i < GetGameState()->PlayerArray.Num(); i++) {
		auto tdmPlayerState = Cast<ATDMPlayerStateBase>(GetGameState()->PlayerArray[i]);
		tdmPlayerState->AddScoreToTotalScore();
		tdmPlayerState->ResetScore();
	}
}

void UTDMScoreHandler::SetCurrentScoresToTotalScore() {
	for(int i = 0; i < GetGameState()->PlayerArray.Num(); i++) {
		auto tdmPlayerState = Cast<ATDMPlayerStateBase>(GetGameState()->PlayerArray[i]);
		tdmPlayerState->SetCurrentScoreToTotalScore();
	}
}

int UTDMScoreHandler::SelectWinnerTeamForRound() {
	int bestScore = -1;
	int bestTeam = 0;
	for(int team = 1; team <= GetGameState()->TeamCount; team++) {
		int score = GetGameState()->GetTeamScore(team);
		if(score > bestScore) {
			bestScore = score;
			bestTeam = team;
		}
	}
	GetGameState()->TeamWins[bestTeam] ++;
	return bestTeam;
}

ATDMGameStateBase* UTDMScoreHandler::GetGameState() {
	auto gameState = GetWorld()->GetGameState<ATDMGameStateBase>();
	AssertNotNull(gameState, GetWorld(), __FILE__, __LINE__);
	return gameState;
}

int UTDMScoreHandler::GetWinningTeam() {
	int winningTeam = 1;
	int maxWins = 0;
	for(int team = 1; team <= GetGameState()->TeamCount; team++) {
		int wins = GetGameState()->TeamWins[team];
		if(wins > maxWins) {
			maxWins = wins;
			winningTeam = team;
		}
	}
	return winningTeam;
}

int UTDMScoreHandler::GetLosingTeam() {
	int losingTeam = 1;
	int minWins = 1000;
	for(int team = 1; team <= GetGameState()->TeamCount; team++) {
		int wins = GetGameState()->TeamWins[team];
		if(wins <= minWins) {
			minWins = wins;
			losingTeam = team;
		}
	}
	return losingTeam;
}