// Fill out your copyright notice in the Description page of Project Settings.

#include "InflectionPoint.h"
#include "Gameplay/CharacterInfoProvider.h"
#include "Gamemodes/TDMGameStateBase.h" 
#include "Gamemodes/TDMPlayerStateBase.h" 
#include "TDMScoreHandler.h"


// Sets default values for this component's properties
UTDMScoreHandler::UTDMScoreHandler() {
}

float UTDMScoreHandler::GetKillerScoreChange(AController * KilledPlayer, AController* KillingPlayer) {
	UCharacterInfoProvider* killerInfo = KillingPlayer ? KillingPlayer->FindComponentByClass<UCharacterInfoProvider>() : NULL;
	UCharacterInfoProvider* killedInfo = KilledPlayer->FindComponentByClass<UCharacterInfoProvider>();

	if(!killerInfo)
		return 0;

	if(IsTeamKill(killedInfo, killerInfo)) {
		return ScorePointsForTeamKill;
	} else {
		if(!killedInfo->IsReplay)
			return ScorePointsForPlayerKill;

		if(killedInfo->IsReplay)
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
	UCharacterInfoProvider* killerInfo = KillingPlayer ? KillingPlayer->FindComponentByClass<UCharacterInfoProvider>() : NULL;
	UCharacterInfoProvider* killedInfo = KilledPlayer->FindComponentByClass<UCharacterInfoProvider>();

	ATDMPlayerStateBase* killedState = Cast<ATDMPlayerStateBase>(killedInfo->PlayerState);
	killedState->Score += GetKilledScoreChange(KilledPlayer, KillingPlayer);

	if(!killerInfo)
		return;
	ATDMPlayerStateBase* killerState = Cast<ATDMPlayerStateBase>(killerInfo->PlayerState);
	killerState->Score += GetKillerScoreChange(KilledPlayer, KillingPlayer);
}


void UTDMScoreHandler::UpdateKillDeathNumbers(AController * KilledPlayer, AController* KillingPlayer) {
	UCharacterInfoProvider* killerInfo = KillingPlayer ? KillingPlayer->FindComponentByClass<UCharacterInfoProvider>() : NULL;
	UCharacterInfoProvider* killedInfo = KilledPlayer->FindComponentByClass<UCharacterInfoProvider>();

	ATDMPlayerStateBase* killedState = Cast<ATDMPlayerStateBase>(killedInfo->PlayerState);
	killedState->Deaths++;
	if(!killerInfo)
		return;

	ATDMPlayerStateBase* killerState = Cast<ATDMPlayerStateBase>(killerInfo->PlayerState);

	if(IsTeamKill(killedInfo, killerInfo)) {
		killerState->TeamKills++;
	} else {
		if(!killedInfo->IsReplay)
			killerState->PlayerKills++;

		if(killedInfo->IsReplay)
			killerState->ReplayKills++;
	}
}

bool UTDMScoreHandler::IsTeamKill(UCharacterInfoProvider* killedInfo, UCharacterInfoProvider* killerInfo) {
	return Cast<ATDMPlayerStateBase>(killedInfo->PlayerState)->Team == Cast<ATDMPlayerStateBase>(killerInfo->PlayerState)->Team;
}


