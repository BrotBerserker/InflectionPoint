// Fill out your copyright notice in the Description page of Project Settings.

#include "InflectionPoint.h"
#include "TDMLevelScriptBase.h"
#include "Runtime/LevelSequence/Public/LevelSequenceActor.h"
#include "Gamemodes/TDMPlayerStateBase.h"



void ATDMLevelScriptBase::MulticastStartSpawnCinematic_Implementation() {
	if(!SoftAssertTrue(SpawnCinematicLevelSequences.Num() > GetTeam(), GetWorld(), __FILE__, __LINE__, "No matching SpawnSequence found in the LevelScript")) {
		return;
	}
	OnPrePlaySequence();
	APlayerController* controller = UGameplayStatics::GetPlayerController(GetWorld(), 0);
	controller->SetViewTargetWithBlend(SpawnCinematicCamera);
	ALevelSequenceActor *sequence = SpawnCinematicLevelSequences[GetTeam()];
	if(!AssertNotNull(sequence, GetWorld(), __FILE__, __LINE__) || !AssertNotNull(sequence->SequencePlayer, GetWorld(), __FILE__, __LINE__))
		return;
	sequence->SequencePlayer->SetPlaybackPosition(0);
	sequence->SequencePlayer->Play();
	sequence->SequencePlayer->OnStop.AddDynamic(this, &ATDMLevelScriptBase::ReturnCameraToPlayer);
}

int ATDMLevelScriptBase::GetTeam() {
	auto state = UGameplayStatics::GetPlayerController(GetWorld(), 0)->PlayerState;
	return Cast<ATDMPlayerStateBase>(state)->Team;
}

void ATDMLevelScriptBase::ReturnCameraToPlayer() {
	APlayerController* controller = UGameplayStatics::GetPlayerController(GetWorld(), 0);
	controller->SetViewTargetWithBlend(controller->GetCharacter());
	OnPostPlaySequence();
}
