// Fill out your copyright notice in the Description page of Project Settings.

#include "InflectionPoint.h"
#include "Gamemodes/TDMPlayerStateBase.h"
#include "TDMGameStateBase.h"
#include "Gameplay/CharacterInfoProvider.h"


void ATDMGameStateBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const {DebugPrint(__FILE__, __LINE__);
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ATDMGameStateBase, CurrentRound);
	DOREPLIFETIME(ATDMGameStateBase, MaxRoundNum);
	DOREPLIFETIME(ATDMGameStateBase, CurrentPhase);
	DOREPLIFETIME(ATDMGameStateBase, MaxPhaseNum);
	DOREPLIFETIME(ATDMGameStateBase, TeamWins);
	DOREPLIFETIME(ATDMGameStateBase, MaxPlayers);
	DOREPLIFETIME(ATDMGameStateBase, NumPlayers);
DebugPrint(__FILE__, __LINE__);}

void ATDMGameStateBase::PrepareForMatchStart(int SpawnPointCount) {DebugPrint(__FILE__, __LINE__);
	TeamWins.Init(0, TeamCount + 1); // +1 because teams start with 1
	CurrentRound = 0;
	CurrentPhase = 0;
	MaxPhaseNum = SpawnPointCount / MaxPlayers;
	ResetPlayerScores();
	ResetTotalPlayerScores();
DebugPrint(__FILE__, __LINE__);}

void ATDMGameStateBase::PrepareForRoundStart() {DebugPrint(__FILE__, __LINE__);
	PreparePlayerStatesForRoundStart();
	CurrentPhase = 0;
	CurrentRound++;
DebugPrint(__FILE__, __LINE__);}

FColor ATDMGameStateBase::GetTeamColor(int Team) {DebugPrint(__FILE__, __LINE__);
	if(Team == 0) {DebugPrint(__FILE__, __LINE__);
		return NeutralColor;
	DebugPrint(__FILE__, __LINE__);}
	ATDMPlayerStateBase* playerState = Cast<ATDMPlayerStateBase>(UGameplayStatics::GetPlayerController(GetWorld(), 0)->PlayerState);
	if(!playerState) {DebugPrint(__FILE__, __LINE__);
		return NeutralColor;
	DebugPrint(__FILE__, __LINE__);}
	return playerState->Team == Team ? FriendlyColor : EnemyColor;
DebugPrint(__FILE__, __LINE__);}

int ATDMGameStateBase::GetTeamScore(int team) {DebugPrint(__FILE__, __LINE__);
	double teamScore = 0;
	for(int i = 0; i < PlayerArray.Num(); i++) {DebugPrint(__FILE__, __LINE__);
		auto tdmPlayerState = Cast<ATDMPlayerStateBase>(PlayerArray[i]);
		AssertNotNull(tdmPlayerState, GetWorld(), __FILE__, __LINE__);
		if(tdmPlayerState && tdmPlayerState->Team == team)
			teamScore += tdmPlayerState->Score;
	DebugPrint(__FILE__, __LINE__);}
	return teamScore;
DebugPrint(__FILE__, __LINE__);}

int ATDMGameStateBase::GetLocalPlayerTeam() {DebugPrint(__FILE__, __LINE__);
	ATDMPlayerStateBase* playerState = Cast<ATDMPlayerStateBase>(UGameplayStatics::GetPlayerController(GetWorld(), 0)->PlayerState);
	if(playerState) {DebugPrint(__FILE__, __LINE__);
		return playerState->Team;
	DebugPrint(__FILE__, __LINE__);}
	return 0;
DebugPrint(__FILE__, __LINE__);}

int ATDMGameStateBase::GetLocalPlayerEnemyTeam() {DebugPrint(__FILE__, __LINE__);
	int team = GetLocalPlayerTeam();
	if(team != 0) {DebugPrint(__FILE__, __LINE__);
		return FMath::Max((team + 1) % (TeamCount + 1), 1);
	DebugPrint(__FILE__, __LINE__);}
	return 0;
DebugPrint(__FILE__, __LINE__);}

void ATDMGameStateBase::ResetPlayerScores() {DebugPrint(__FILE__, __LINE__);
	for(int i = 0; i < PlayerArray.Num(); i++) {DebugPrint(__FILE__, __LINE__);
		auto tdmPlayerState = Cast<ATDMPlayerStateBase>(PlayerArray[i]);
		AssertNotNull(tdmPlayerState, GetWorld(), __FILE__, __LINE__);
		tdmPlayerState->ResetScore();
	DebugPrint(__FILE__, __LINE__);}
DebugPrint(__FILE__, __LINE__);}

void ATDMGameStateBase::ResetTotalPlayerScores() {DebugPrint(__FILE__, __LINE__);
	for(int i = 0; i < PlayerArray.Num(); i++) {DebugPrint(__FILE__, __LINE__);
		auto tdmPlayerState = Cast<ATDMPlayerStateBase>(PlayerArray[i]);
		AssertNotNull(tdmPlayerState, GetWorld(), __FILE__, __LINE__);
		tdmPlayerState->ResetTotalScore();
	DebugPrint(__FILE__, __LINE__);}
DebugPrint(__FILE__, __LINE__);}

void ATDMGameStateBase::PreparePlayerStatesForRoundStart() {DebugPrint(__FILE__, __LINE__);
	for(int i = 0; i < PlayerArray.Num(); i++) {DebugPrint(__FILE__, __LINE__);
		auto tdmPlayerState = Cast<ATDMPlayerStateBase>(PlayerArray[i]);
		AssertNotNull(tdmPlayerState, GetWorld(), __FILE__, __LINE__);
		tdmPlayerState->PrepareForRoundStart();
	DebugPrint(__FILE__, __LINE__);}
DebugPrint(__FILE__, __LINE__);}
