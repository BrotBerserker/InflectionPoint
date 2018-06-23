// Fill out your copyright notice in the Description page of Project Settings.

#include "InflectionPoint.h"
#include "TDMLevelScriptBase.h"
#include "Gameplay/Characters/BaseCharacter.h"
#include "Runtime/LevelSequence/Public/LevelSequenceActor.h"
#include "Gamemodes/TDMPlayerStateBase.h"


void ATDMLevelScriptBase::MulticastStartSpawnCinematic_Implementation() {
	if(!SoftAssertTrue(SpawnCinematicLevelSequences.Num() > GetTeam(), GetWorld(), __FILE__, __LINE__, "No matching SpawnSequence found in the LevelScript")) {
		return;
	}
	OnPrePlaySpawnCinematic();
	PlaySequence(SpawnCinematicCamera, SpawnCinematicLevelSequences[GetTeam()]);
}

void ATDMLevelScriptBase::StartEndMatchSequence(TSubclassOf<AActor> WinningActor, TSubclassOf<AActor> LosingActor) {
	if(SoftAssertTrue(WinningPlayerLocation != nullptr && LosingPlayerLocation != nullptr, GetWorld(), __FILE__, __LINE__, "No locations for winning/losing player provided!")) {
		AActor* winner = SpawnActorForEndMatchSequence(WinningActor, WinningPlayerLocation);
		AActor* loser = SpawnActorForEndMatchSequence(LosingActor, LosingPlayerLocation);
		PlayEndMatchSequenceAnimation(winner, WinningPlayerAnimation);
		PlayEndMatchSequenceAnimation(loser, LosingPlayerAnimation);
	}
	MulticastStartEndMatchSequence();
}

void ATDMLevelScriptBase::MulticastStartEndMatchSequence_Implementation() {
	PlaySequence(MatchEndCamera, MatchEndLevelSequence);
}

void ATDMLevelScriptBase::PlaySequence(ACameraActor* camera, ALevelSequenceActor* sequenceActor) {
	if(!SoftAssertTrue(sequenceActor != nullptr, GetWorld(), __FILE__, __LINE__, "No level sequence provided!"))
		return;
	APlayerController* controller = UGameplayStatics::GetPlayerController(GetWorld(), 0);
	controller->SetViewTargetWithBlend(camera);
	sequenceActor->SequencePlayer->SetPlaybackPosition(0);
	sequenceActor->SequencePlayer->Play();
	sequenceActor->SequencePlayer->OnStop.AddDynamic(this, &ATDMLevelScriptBase::ReturnCameraToPlayer);
}

AActor* ATDMLevelScriptBase::SpawnActorForEndMatchSequence(TSubclassOf<AActor> actorToSpawn, AActor* location) {
	FVector loc = location->GetTransform().GetLocation();
	FRotator rot = FRotator(location->GetTransform().GetRotation());

	FActorSpawnParameters ActorSpawnParams;
	ActorSpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	return GetWorld()->SpawnActor<AActor>(actorToSpawn, loc, rot, ActorSpawnParams);
}

void ATDMLevelScriptBase::PlayEndMatchSequenceAnimation(AActor * spawnedActor, UAnimationAsset* animation) {
	ABaseCharacter* baseCharacter = Cast<ABaseCharacter>(spawnedActor);
	if(!baseCharacter) {
		return;
	}
	baseCharacter->ServerEquipRandomWeapon();
	baseCharacter->MulticastPlay3PAnimation(animation);
}

int ATDMLevelScriptBase::GetTeam() {
	auto state = UGameplayStatics::GetPlayerController(GetWorld(), 0)->PlayerState;
	return Cast<ATDMPlayerStateBase>(state)->Team;
}

void ATDMLevelScriptBase::ReturnCameraToPlayer() {
	APlayerController* controller = UGameplayStatics::GetPlayerController(GetWorld(), 0);
	controller->SetViewTargetWithBlend(controller->GetCharacter());
}
