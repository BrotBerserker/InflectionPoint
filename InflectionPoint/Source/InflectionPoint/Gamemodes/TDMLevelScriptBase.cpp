// Fill out your copyright notice in the Description page of Project Settings.

#include "InflectionPoint.h"
#include "TDMLevelScriptBase.h"
#include "Gameplay/Characters/BaseCharacter.h"
#include "Runtime/LevelSequence/Public/LevelSequenceActor.h"
#include "Gameplay/Characters/PlayerCharacterBase.h"
#include "Gameplay/Characters/ReplayCharacterBase.h"
#include "Level/ResettableLevelActor.h"
#include "Gamemodes/TDMPlayerStateBase.h"


void ATDMLevelScriptBase::MulticastStartSpawnCinematic_Implementation() {DebugPrint(__FILE__, __LINE__);
	if(!SoftAssertTrue(SpawnCinematicLevelSequences.Num() > GetTeam() && GetTeam() >= 0, GetWorld(), __FILE__, __LINE__, "SpawnSequence could not be played!")) {DebugPrint(__FILE__, __LINE__);
		return;
	DebugPrint(__FILE__, __LINE__);}
	OnPrePlaySpawnCinematic();
	PrepareAndStartSequence(SpawnCinematicCamera, SpawnCinematicLevelSequences[GetTeam()], 0.3f);
DebugPrint(__FILE__, __LINE__);}

void ATDMLevelScriptBase::StartEndMatchSequence(TSubclassOf<AActor> WinningActor, TSubclassOf<AActor> LosingActor, FString WinnerName, FString LoserName) {DebugPrint(__FILE__, __LINE__);
	if(SoftAssertTrue(WinningPlayerLocation != nullptr && LosingPlayerLocation != nullptr, GetWorld(), __FILE__, __LINE__, "No locations for winning/losing player provided!")) {DebugPrint(__FILE__, __LINE__);
		AActor* winner = SpawnActorForEndMatchSequence(WinningActor, WinningPlayerLocation);
		AActor* loser = SpawnActorForEndMatchSequence(LosingActor, LosingPlayerLocation);
		PrepareActorForEndMatchSequence(winner, WinningPlayerAnimation);
		PrepareActorForEndMatchSequence(loser, LosingPlayerAnimation);
	DebugPrint(__FILE__, __LINE__);}
	MulticastStartEndMatchSequence(WinnerName, LoserName);
DebugPrint(__FILE__, __LINE__);}

void ATDMLevelScriptBase::MulticastStartEndMatchSequence_Implementation(const FString& winnerName, const FString& loserName) {DebugPrint(__FILE__, __LINE__);
	// TODO disable HUD
	UpdateNameTag(WinningPlayerLocation, winnerName);
	UpdateNameTag(LosingPlayerLocation, loserName);
	PrepareAndStartSequence(MatchEndCamera, MatchEndLevelSequence, 1.f);
DebugPrint(__FILE__, __LINE__);}

void ATDMLevelScriptBase::PrepareAndStartSequence(ACameraActor* camera, ALevelSequenceActor* sequenceActor, float fadeTime) {DebugPrint(__FILE__, __LINE__);
	if(!SoftAssertTrue(sequenceActor != nullptr, GetWorld(), __FILE__, __LINE__, "No level sequence provided!"))
		return;
	APlayerController* controller = UGameplayStatics::GetPlayerController(GetWorld(), 0);
	controller->ClientSetCameraFade(true, FColor::Black, FVector2D(0, 1), fadeTime + 0.1f);
	StartTimer(this, GetWorld(), "StartSequence", fadeTime, false, controller, camera, sequenceActor, fadeTime);
DebugPrint(__FILE__, __LINE__);}

void ATDMLevelScriptBase::StartSequence(APlayerController* controller, ACameraActor* camera, ALevelSequenceActor* sequenceActor, float fadeTime) {DebugPrint(__FILE__, __LINE__);
	controller->ClientSetCameraFade(true, FColor::Black, FVector2D(1, 0), fadeTime);
	controller->SetViewTargetWithBlend(camera);
	sequenceActor->SequencePlayer->PlayToSeconds(0);
	sequenceActor->SequencePlayer->Play();
	sequenceActor->SequencePlayer->OnStop.AddDynamic(this, &ATDMLevelScriptBase::CleanUpAfterSequence);
DebugPrint(__FILE__, __LINE__);}

AActor* ATDMLevelScriptBase::SpawnActorForEndMatchSequence(TSubclassOf<AActor> actorToSpawn, AActor* location) {DebugPrint(__FILE__, __LINE__);
	FVector loc = location->GetTransform().GetLocation();
	FRotator rot = FRotator(location->GetTransform().GetRotation());

	FActorSpawnParameters ActorSpawnParams;
	ActorSpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	return GetWorld()->SpawnActor<AActor>(actorToSpawn, loc, rot, ActorSpawnParams);
DebugPrint(__FILE__, __LINE__);}

void ATDMLevelScriptBase::PrepareActorForEndMatchSequence(AActor * spawnedActor, UAnimationAsset* animation) {DebugPrint(__FILE__, __LINE__);
	ABaseCharacter* baseCharacter = Cast<ABaseCharacter>(spawnedActor);
	if(baseCharacter) {DebugPrint(__FILE__, __LINE__);
		baseCharacter->ServerEquipRandomWeapon();
		baseCharacter->MulticastPlay3PAnimation(animation);
	DebugPrint(__FILE__, __LINE__);}
DebugPrint(__FILE__, __LINE__);}

void ATDMLevelScriptBase::UpdateNameTag(AActor* location, FString name) {DebugPrint(__FILE__, __LINE__);
	if(!location) {DebugPrint(__FILE__, __LINE__);
		return;
	DebugPrint(__FILE__, __LINE__);}
	UTextRenderComponent* textRender = location->FindComponentByClass<UTextRenderComponent>();
	if(textRender) {DebugPrint(__FILE__, __LINE__);
		textRender->SetText(FText::FromString(name));
	DebugPrint(__FILE__, __LINE__);}
DebugPrint(__FILE__, __LINE__);}

int ATDMLevelScriptBase::GetTeam() {DebugPrint(__FILE__, __LINE__);
	auto controller = UGameplayStatics::GetPlayerController(GetWorld(), 0);
	if(!controller) {DebugPrint(__FILE__, __LINE__);
		return -1;
	DebugPrint(__FILE__, __LINE__);}
	auto state = Cast<ATDMPlayerStateBase>(controller->PlayerState);
	if(!state) {DebugPrint(__FILE__, __LINE__);
		return -1;
	DebugPrint(__FILE__, __LINE__);}
	return state->Team;
DebugPrint(__FILE__, __LINE__);}

void ATDMLevelScriptBase::CleanUpAfterSequence() {DebugPrint(__FILE__, __LINE__);
	APlayerController* controller = UGameplayStatics::GetPlayerController(GetWorld(), 0);
	controller->SetViewTargetWithBlend(controller->GetCharacter());
	UpdateNameTag(WinningPlayerLocation, "");
	UpdateNameTag(LosingPlayerLocation, "");
DebugPrint(__FILE__, __LINE__);}

void ATDMLevelScriptBase::ResetLevel() {DebugPrint(__FILE__, __LINE__);
	DestroyAllActors(AReplayCharacterBase::StaticClass());
	DestroyAllActors(APlayerCharacterBase::StaticClass());
	DestroyAllActors(ABaseWeapon::StaticClass());
	DestroyAllActors(AInflectionPointProjectile::StaticClass());
	DestroyAllActorsWithTag(FName("DeleteOnClearMap"));
	ResetAllLevelActors();
DebugPrint(__FILE__, __LINE__);}

void ATDMLevelScriptBase::ResetAllLevelActors() {DebugPrint(__FILE__, __LINE__);
	TArray<AActor*> foundActors;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), AResettableLevelActor::StaticClass(), foundActors);
	for(auto& item : foundActors)
		Cast<AResettableLevelActor>(item)->ResetLevelActor();
DebugPrint(__FILE__, __LINE__);}

void ATDMLevelScriptBase::DestroyAllActors(TSubclassOf<AActor> actorClass) {DebugPrint(__FILE__, __LINE__);
	TArray<AActor*> foundActors;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), actorClass, foundActors);
	for(auto& item : foundActors)
		item->Destroy();
DebugPrint(__FILE__, __LINE__);}

void ATDMLevelScriptBase::DestroyAllActorsWithTag(FName tag) {DebugPrint(__FILE__, __LINE__);
	TArray<AActor*> foundActors;
	UGameplayStatics::GetAllActorsWithTag(GetWorld(), tag, foundActors);
	for(auto& item : foundActors)
		item->Destroy();
DebugPrint(__FILE__, __LINE__);}