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
	PrepareAndStartSequence(SpawnCinematicCamera, SpawnCinematicLevelSequences[GetTeam()], 0.3f);
}

void ATDMLevelScriptBase::StartEndMatchSequence(TSubclassOf<AActor> WinningActor, TSubclassOf<AActor> LosingActor, FString WinnerName, FString LoserName) {
	if(SoftAssertTrue(WinningPlayerLocation != nullptr && LosingPlayerLocation != nullptr, GetWorld(), __FILE__, __LINE__, "No locations for winning/losing player provided!")) {
		AActor* winner = SpawnActorForEndMatchSequence(WinningActor, WinningPlayerLocation);
		AActor* loser = SpawnActorForEndMatchSequence(LosingActor, LosingPlayerLocation);
		PrepareActorForEndMatchSequence(winner, WinningPlayerAnimation);
		PrepareActorForEndMatchSequence(loser, LosingPlayerAnimation);
	}
	MulticastStartEndMatchSequence(WinnerName, LoserName);
}

void ATDMLevelScriptBase::MulticastStartEndMatchSequence_Implementation(const FString& winnerName, const FString& loserName) {
	// TODO disable HUD
	UpdateNameTag(WinningPlayerLocation, winnerName);
	UpdateNameTag(LosingPlayerLocation, loserName);
	PrepareAndStartSequence(MatchEndCamera, MatchEndLevelSequence, 1.f);
}

void ATDMLevelScriptBase::PrepareAndStartSequence(ACameraActor* camera, ALevelSequenceActor* sequenceActor, float fadeTime) {
	if(!SoftAssertTrue(sequenceActor != nullptr, GetWorld(), __FILE__, __LINE__, "No level sequence provided!"))
		return;
	APlayerController* controller = UGameplayStatics::GetPlayerController(GetWorld(), 0);
	controller->ClientSetCameraFade(true, FColor::Black, FVector2D(0, 1), fadeTime + 0.1f);
	StartTimer(this, GetWorld(), "StartSequence", fadeTime, false, controller, camera, sequenceActor, fadeTime);
}

void ATDMLevelScriptBase::StartSequence(APlayerController* controller, ACameraActor* camera, ALevelSequenceActor* sequenceActor, float fadeTime) {
	controller->ClientSetCameraFade(true, FColor::Black, FVector2D(1, 0), fadeTime);
	controller->SetViewTargetWithBlend(camera);
	sequenceActor->SequencePlayer->SetPlaybackPosition(0);
	sequenceActor->SequencePlayer->Play();
	sequenceActor->SequencePlayer->OnStop.AddDynamic(this, &ATDMLevelScriptBase::CleanUpAfterSequence);
}

AActor* ATDMLevelScriptBase::SpawnActorForEndMatchSequence(TSubclassOf<AActor> actorToSpawn, AActor* location) {
	FVector loc = location->GetTransform().GetLocation();
	FRotator rot = FRotator(location->GetTransform().GetRotation());

	FActorSpawnParameters ActorSpawnParams;
	ActorSpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	return GetWorld()->SpawnActor<AActor>(actorToSpawn, loc, rot, ActorSpawnParams);
}

void ATDMLevelScriptBase::PrepareActorForEndMatchSequence(AActor * spawnedActor, UAnimationAsset* animation) {
	ABaseCharacter* baseCharacter = Cast<ABaseCharacter>(spawnedActor);
	if(baseCharacter) {
		baseCharacter->ServerEquipRandomWeapon();
		baseCharacter->MulticastPlay3PAnimation(animation);
	}
}

void ATDMLevelScriptBase::UpdateNameTag(AActor* location, FString name) {
	if(!location) {
		return;
	}
	UTextRenderComponent* textRender = location->FindComponentByClass<UTextRenderComponent>();
	if(textRender) {
		textRender->SetText(FText::FromString(name));
	}
}

int ATDMLevelScriptBase::GetTeam() {
	auto state = UGameplayStatics::GetPlayerController(GetWorld(), 0)->PlayerState;
	return Cast<ATDMPlayerStateBase>(state)->Team;
}

void ATDMLevelScriptBase::CleanUpAfterSequence() {
	APlayerController* controller = UGameplayStatics::GetPlayerController(GetWorld(), 0);
	controller->SetViewTargetWithBlend(controller->GetCharacter());
	UpdateNameTag(WinningPlayerLocation, "");
	UpdateNameTag(LosingPlayerLocation, "");
}
