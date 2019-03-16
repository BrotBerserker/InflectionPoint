// Fill out your copyright notice in the Description page of Project Settings.

#include "InflectionPoint.h"
#include "Gamemodes/TDMPlayerStateBase.h"
#include "TDMGameStateBase.h"
#include "Gameplay/CharacterInfoProvider.h"


void ATDMGameStateBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const {
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ATDMGameStateBase, CurrentRound);
	DOREPLIFETIME(ATDMGameStateBase, MaxRoundNum);
	DOREPLIFETIME(ATDMGameStateBase, CurrentPhase);
	DOREPLIFETIME(ATDMGameStateBase, MaxPhaseNum);
	DOREPLIFETIME(ATDMGameStateBase, TeamWins);
	DOREPLIFETIME(ATDMGameStateBase, MaxPlayers);
	DOREPLIFETIME(ATDMGameStateBase, NumPlayers);
	DOREPLIFETIME(ATDMGameStateBase, localPhaseStartGameTime);	
}

void ATDMGameStateBase::PrepareForMatchStart(int SpawnPointCount) {
	TeamWins.Init(0, TeamCount + 1); // +1 because teams start with 1
	CurrentRound = 0;
	CurrentPhase = 0;
	MaxPhaseNum = SpawnPointCount / MaxPlayers;
	ResetPlayerScores();
	ResetTotalPlayerScores();
}

void ATDMGameStateBase::PrepareForRoundStart() {
	PreparePlayerStatesForRoundStart();
	CurrentPhase = 0;
	CurrentRound++;
}

FColor ATDMGameStateBase::GetTeamColor(int Team) {
	if(Team == 0) {
		return NeutralColor;
	}
	ATDMPlayerStateBase* playerState = Cast<ATDMPlayerStateBase>(UGameplayStatics::GetPlayerController(GetWorld(), 0)->PlayerState);
	if(!playerState) {
		return NeutralColor;
	}
	return playerState->Team == Team ? FriendlyColor : EnemyColor;
}

int ATDMGameStateBase::GetTeamScore(int team) {
	double teamScore = 0;
	for(int i = 0; i < PlayerArray.Num(); i++) {
		auto tdmPlayerState = Cast<ATDMPlayerStateBase>(PlayerArray[i]);
		AssertNotNull(tdmPlayerState, GetWorld(), __FILE__, __LINE__);
		if(tdmPlayerState && tdmPlayerState->Team == team)
			teamScore += tdmPlayerState->Score;
	}
	return teamScore;
}

int ATDMGameStateBase::GetLocalPlayerTeam() {
	ATDMPlayerStateBase* playerState = Cast<ATDMPlayerStateBase>(UGameplayStatics::GetPlayerController(GetWorld(), 0)->PlayerState);
	if(playerState) {
		return playerState->Team;
	}
	return 0;
}

int ATDMGameStateBase::GetLocalPlayerEnemyTeam() {
	int team = GetLocalPlayerTeam();
	if(team != 0) {
		return FMath::Max((team + 1) % (TeamCount + 1), 1);
	}
	return 0;
}

void ATDMGameStateBase::ResetPlayerScores() {
	for(int i = 0; i < PlayerArray.Num(); i++) {
		auto tdmPlayerState = Cast<ATDMPlayerStateBase>(PlayerArray[i]);
		AssertNotNull(tdmPlayerState, GetWorld(), __FILE__, __LINE__);
		tdmPlayerState->ResetScore();
	}
}

void ATDMGameStateBase::ResetTotalPlayerScores() {
	for(int i = 0; i < PlayerArray.Num(); i++) {
		auto tdmPlayerState = Cast<ATDMPlayerStateBase>(PlayerArray[i]);
		AssertNotNull(tdmPlayerState, GetWorld(), __FILE__, __LINE__);
		tdmPlayerState->ResetTotalScore();
	}
}

void ATDMGameStateBase::PreparePlayerStatesForRoundStart() {
	for(int i = 0; i < PlayerArray.Num(); i++) {
		auto tdmPlayerState = Cast<ATDMPlayerStateBase>(PlayerArray[i]);
		AssertNotNull(tdmPlayerState, GetWorld(), __FILE__, __LINE__);
		tdmPlayerState->PrepareForRoundStart();
	}
}

void ATDMGameStateBase::MulticastStartPhase_Implementation() {
	localPhaseStartGameTime = UGameplayStatics::GetRealTimeSeconds(GetWorld());
}
