// Fill out your copyright notice in the Description page of Project Settings.

#include "InflectionPoint.h"
#include "TDMPlayerStateBase.h"




void ATDMPlayerStateBase::GetLifetimeReplicatedProps(TArray< FLifetimeProperty > & OutLifetimeProps) const {
	DOREPLIFETIME(ATDMPlayerStateBase, Team);
	DOREPLIFETIME(ATDMPlayerStateBase, PlayerStartGroup);
	DOREPLIFETIME(ATDMPlayerStateBase, PlayerName);
	DOREPLIFETIME(ATDMPlayerStateBase, PlayerKills);
	DOREPLIFETIME(ATDMPlayerStateBase, ReplayKills);
	DOREPLIFETIME(ATDMPlayerStateBase, Deaths);
}

void ATDMPlayerStateBase::AddPlayerKill() {
	if(!HasAuthority())
		return;
	Score += 100;
	PlayerKills++;
}
void ATDMPlayerStateBase::AddReplayKill() {
	if(!HasAuthority())
		return;
	Score += 50;
	ReplayKills++;
}
void ATDMPlayerStateBase::AddDeath() {
	if(!HasAuthority())
		return;
	Deaths++;
}

int ATDMPlayerStateBase::GetPlayerKills() {
	return PlayerKills;
}
int ATDMPlayerStateBase::GetReplayKills() {
	return ReplayKills;
}
int ATDMPlayerStateBase::GetDeaths() {
	return Deaths;
}