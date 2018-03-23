// Fill out your copyright notice in the Description page of Project Settings.

#include "InflectionPoint.h"
#include "Gameplay/CharacterInfoProvider.h"
#include "Gamemodes/TDMGameStateBase.h" 
#include "Gamemodes/TDMPlayerStateBase.h" 
#include "TDMScoreHandler.h"


// Sets default values for this component's properties
UTDMScoreHandler::UTDMScoreHandler() {DebugPrint(__FILE__, __LINE__);
DebugPrint(__FILE__, __LINE__);}

float UTDMScoreHandler::GetKillerScoreChange(AController * KilledPlayer, AController* KillingPlayer) {DebugPrint(__FILE__, __LINE__);
	UCharacterInfoProvider* killerInfo = KillingPlayer ? KillingPlayer->GetCharacter()->FindComponentByClass<UCharacterInfoProvider>() : NULL;
	UCharacterInfoProvider* killedInfo = KilledPlayer->GetCharacter()->FindComponentByClass<UCharacterInfoProvider>();

	if(!killerInfo)
		return 0;

	if(IsTeamKill(killedInfo, killerInfo)) {DebugPrint(__FILE__, __LINE__);
		return ScorePointsForTeamKill;
	DebugPrint(__FILE__, __LINE__);} else {DebugPrint(__FILE__, __LINE__);
		if(!killedInfo->IsReplay)
			return ScorePointsForPlayerKill;

		if(killedInfo->IsReplay)
			return ScorePointsForReplayKill;
	DebugPrint(__FILE__, __LINE__);}
	return 0;
DebugPrint(__FILE__, __LINE__);}

float UTDMScoreHandler::GetKilledScoreChange(AController * KilledPlayer, AController* KillingPlayer) {DebugPrint(__FILE__, __LINE__);
	return ScorePointsForDeath;
DebugPrint(__FILE__, __LINE__);}


void UTDMScoreHandler::AddKill(AController * KilledPlayer, AController* KillingPlayer) {DebugPrint(__FILE__, __LINE__);
	UpdatePlayerScoreAfterKill(KilledPlayer, KillingPlayer);
	UpdateKillDeathNumbers(KilledPlayer, KillingPlayer);
DebugPrint(__FILE__, __LINE__);}

void UTDMScoreHandler::UpdatePlayerScoreAfterKill(AController * KilledPlayer, AController* KillingPlayer) {DebugPrint(__FILE__, __LINE__);
	UCharacterInfoProvider* killerInfo = KillingPlayer ? KillingPlayer->GetCharacter()->FindComponentByClass<UCharacterInfoProvider>() : NULL;
	UCharacterInfoProvider* killedInfo = KilledPlayer->GetCharacter()->FindComponentByClass<UCharacterInfoProvider>();

	ATDMPlayerStateBase* killedState = Cast<ATDMPlayerStateBase>(killedInfo->PlayerState);
	killedState->Score += GetKilledScoreChange(KilledPlayer, KillingPlayer);

	if(!killerInfo)
		return;
	ATDMPlayerStateBase* killerState = Cast<ATDMPlayerStateBase>(killerInfo->PlayerState);
	killerState->Score += GetKillerScoreChange(KilledPlayer, KillingPlayer);
DebugPrint(__FILE__, __LINE__);}


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
	DebugPrint(__FILE__, __LINE__);} else {DebugPrint(__FILE__, __LINE__);
		if(!killedInfo->IsReplay)
			killerState->PlayerKills++;

		if(killedInfo->IsReplay)
			killerState->ReplayKills++;
	DebugPrint(__FILE__, __LINE__);}
DebugPrint(__FILE__, __LINE__);}

bool UTDMScoreHandler::IsTeamKill(UCharacterInfoProvider* killedInfo, UCharacterInfoProvider* killerInfo) {DebugPrint(__FILE__, __LINE__);
	return Cast<ATDMPlayerStateBase>(killedInfo->PlayerState)->Team == Cast<ATDMPlayerStateBase>(killerInfo->PlayerState)->Team;
DebugPrint(__FILE__, __LINE__);}


