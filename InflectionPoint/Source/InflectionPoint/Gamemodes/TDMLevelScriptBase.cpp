// Fill out your copyright notice in the Description page of Project Settings.

#include "InflectionPoint.h"
#include "TDMLevelScriptBase.h"
#include "Runtime/LevelSequence/Public/LevelSequenceActor.h"
#include "Gamemodes/TDMPlayerStateBase.h"



void ATDMLevelScriptBase::MulticastStartSpawnCinematic_Implementation() {
	if(!SoftAssertTrue(SpawnCinematicLevelSequences.Num() > GetTeam(), GetWorld(), __FILE__, __LINE__, "No matching SpawnSequence found in the LevelScript")) {
		return;
	}
	ALevelSequenceActor *sequence = SpawnCinematicLevelSequences[GetTeam()];
	AssertNotNull(sequence, GetWorld(), __FILE__, __LINE__);
	AssertNotNull(sequence->SequencePlayer, GetWorld(), __FILE__, __LINE__);
	sequence->SequencePlayer->SetPlaybackPosition(0);
	sequence->SequencePlayer->Play();
}

int ATDMLevelScriptBase::GetTeam() {
	auto state = UGameplayStatics::GetPlayerCharacter(GetWorld(), 0)->Controller->PlayerState;
	return Cast<ATDMPlayerStateBase>(state)->Team;
}
