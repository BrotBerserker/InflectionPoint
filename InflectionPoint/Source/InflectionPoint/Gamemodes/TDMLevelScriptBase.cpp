// Fill out your copyright notice in the Description page of Project Settings.

#include "InflectionPoint.h"
#include "TDMLevelScriptBase.h"
#include "Runtime/LevelSequence/Public/LevelSequenceActor.h"
#include "Gamemodes/TDMPlayerStateBase.h"



void ATDMLevelScriptBase::MulticastStartSpawnCinematic_Implementation() {
	auto searchTag = FString("CinematicSpawn");
	searchTag.AppendInt(GetTeam());
	UE_LOG(LogTemp, Warning, TEXT("Searching for team [%s]"), *(searchTag));
	for(TActorIterator<ALevelSequenceActor> ActorItr(GetWorld()); ActorItr; ++ActorItr) {
		// Same as with the Object Iterator, access the subclass instance with the * or -> operators.
		ALevelSequenceActor *sequence = *ActorItr;
		if(sequence->ActorHasTag(FName(*searchTag))) {
			UE_LOG(LogTemp, Warning, TEXT("play cinematic"));
			sequence->SequencePlayer->Play();
		}
	}
}

int ATDMLevelScriptBase::GetTeam() {
	auto state = UGameplayStatics::GetPlayerCharacter(GetWorld(), 0)->Controller->PlayerState;
	return Cast<ATDMPlayerStateBase>(state)->Team;
}
