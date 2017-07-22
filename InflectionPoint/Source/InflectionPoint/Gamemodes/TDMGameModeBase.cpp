// Copyright 1998-2016 Epic Games, Inc. All Rights Reserved.

#include "InflectionPoint.h"
#include "TDMGameModeBase.h"
#include <string>
#include "UI/HUD/InflectionPointHUD.h"
#include "Gameplay/Characters/BaseCharacter.h"
#include "Gameplay/Damage/MortalityProvider.h"
#include "Gameplay/Weapons/InflectionPointProjectile.h"
#include "Gameplay/Controllers/InflectionPointPlayerController.h"
#include "Gameplay/Characters/PlayerControlledFPSCharacter.h"
#include "Gameplay/Characters/ReplayControlledFPSCharacter.h"

ATDMGameModeBase::ATDMGameModeBase()
	: Super() {
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnClassFinder(TEXT("/Game/InflectionPoint/Blueprints/Characters/PlayerCharacter"));
	DefaultPawnClass = PlayerPawnClassFinder.Class;

	// use our custom HUD class
	HUDClass = AInflectionPointHUD::StaticClass();
	PlayerControllerClass = AInflectionPointPlayerController::StaticClass();
	GameStateClass = ATDMGameState::StaticClass();
}

void ATDMGameModeBase::UpdateMaxPlayers(FName SessioName) {
	IOnlineSessionPtr session = IOnlineSubsystem::Get()->GetSessionInterface();
	FOnlineSessionSettings* sessionSettings = session->GetSessionSettings(SessioName);
	if(sessionSettings) {
		MaxPlayers = sessionSettings->NumPublicConnections;
	} else {
		UE_LOG(LogTemp, Warning, TEXT("Warning: No session settings could be found, setting MaxPlayers to %d."), OfflineMaxPlayers);
		MaxPlayers = OfflineMaxPlayers;
	}
}

void ATDMGameModeBase::StartMatch() {
	GetGameState()->CurrentRound = 0;
	AssignTeamsAndPlayerStartGroups();
	StartNextRound();
}

void ATDMGameModeBase::StartNextRound() {
	// Save replays from players that are stil alive
	SaveRecordingsFromRemainingPlayers();
	int round = GetGameState()->CurrentRound + 1;
	if(round > GetGameState()->MaxRoundNum)
		round = 1; // restart 
	GetGameState()->CurrentRound = round;
	ClearMap();
	SpawnPlayersAndReplays();
}

void ATDMGameModeBase::PlayerDied(AInflectionPointPlayerController * playerController) {
	if(GetGameState()->CurrentRound > 0)
		SavePlayerRecordings(playerController);

	if(GetGameState()->CurrentRound == 0) {
		SpawnAndPossessPlayer(playerController);
	} else if(IsRoundFinished()) {
		StartNextRound();
	} else {
		// TODO: Set Player as spectator
	}
}

bool ATDMGameModeBase::IsRoundFinished() {
	return GetTeamsAlive().Num() <= 1;
}

TArray<int> ATDMGameModeBase::GetTeamsAlive() {
	TArray<int> teamsAlive;
	for(FConstPlayerControllerIterator Iterator = GetWorld()->GetPlayerControllerIterator(); Iterator; ++Iterator) {
		auto playerController = UGameplayStatics::GetPlayerController(GetWorld(), Iterator.GetIndex());
		auto ipPlayerController = Cast<AInflectionPointPlayerController>(playerController);
		if(teamsAlive.Contains(ipPlayerController->Team))
			continue;
		if(IsPlayerAlive(ipPlayerController))
			teamsAlive.Add(ipPlayerController->Team);
	}
	return teamsAlive;
}


bool ATDMGameModeBase::IsPlayerAlive(AInflectionPointPlayerController* playerController) {
	auto pawn = playerController->GetPawn();
	if(!pawn)
		return false;

	auto mortalityProvider = pawn->FindComponentByClass<UMortalityProvider>();
	if(mortalityProvider && mortalityProvider->CurrentHealth > 0)
		return true;

	return false;
}

void ATDMGameModeBase::SpawnPlayersAndReplays() {
	for(FConstPlayerControllerIterator Iterator = GetWorld()->GetPlayerControllerIterator(); Iterator; ++Iterator) {
		auto playerController = UGameplayStatics::GetPlayerController(GetWorld(), Iterator.GetIndex());
		auto ipPlayerController = Cast<AInflectionPointPlayerController>(playerController);
		SpawnAndPossessPlayer(ipPlayerController);
		for(int i = 1; i < GetGameState()->CurrentRound; i++)
			SpawnAndPrepareReplay(ipPlayerController, i);
	}
}

template <typename CharacterType>
CharacterType* ATDMGameModeBase::SpawnCharacter(UClass* spawnClass, AInflectionPointPlayerController * playerController, AActor* playerStart) {
	FVector loc = playerStart->GetTransform().GetLocation();
	FRotator rot = FRotator(playerStart->GetTransform().GetRotation());

	FActorSpawnParameters ActorSpawnParams;
	ActorSpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	CharacterType* newCharacter = GetWorld()->SpawnActor<CharacterType>(spawnClass, loc, rot, ActorSpawnParams);
	AssertNotNull(newCharacter, GetWorld(), __FILE__, __LINE__, "Could not spawn character!");
	return newCharacter;
}

void ATDMGameModeBase::SpawnAndPossessPlayer(AInflectionPointPlayerController * playerController) {
	AActor* spawnPoint = FindSpawnForPlayer(playerController, GetGameState()->CurrentRound);
	AssertNotNull(spawnPoint, GetWorld(), __FILE__, __LINE__);

	auto character = SpawnCharacter<APlayerControlledFPSCharacter>(DefaultPawnClass.Get(), playerController, spawnPoint);

	playerController->ClientSetControlRotation(FRotator(spawnPoint->GetTransform().GetRotation()));
	playerController->Possess(character);

	if(GetGameState()->CurrentRound > 0)
		StartCountdown(character);
}

void ATDMGameModeBase::SpawnAndPrepareReplay(AInflectionPointPlayerController* playerController, int round) {
	AssertTrue(PlayerRecordings.Contains(playerController), GetWorld(), __FILE__, __LINE__, "Could not find replay");
	auto spawnPoint = FindSpawnForPlayer(playerController, round);
	AssertNotNull(spawnPoint, GetWorld(), __FILE__, __LINE__, "No spawn found");
	auto character = SpawnCharacter<AReplayControlledFPSCharacter>(ReplayCharacter, playerController, spawnPoint);

	AssertTrue(PlayerRecordings[playerController].Contains(round), GetWorld(), __FILE__, __LINE__, "Could not find replay");

	// Start Replay on spawned ReplayCharacter
	if(PlayerRecordings[playerController].Contains(round))
		character->SetReplayData(PlayerRecordings[playerController][round]);
}

void ATDMGameModeBase::StartCountdown(APlayerControlledFPSCharacter * newCharacter) {
	newCharacter->ClientSetIgnoreInput(true);

	for(int i = CountDownDuration; i >= 0; i--) {
		StartTimer(this, GetWorld(), "UpdateCountdown", (CountDownDuration - i + 1), false, newCharacter, i);
	}

	StartTimer(this, GetWorld(), "StartReplays", CountDownDuration + 1, false);
}

void ATDMGameModeBase::UpdateCountdown(APlayerControlledFPSCharacter* character, int number) {
	character->ClientShowCountdownNumber(number);
	if(number == 0) {
		character->ClientStartRecording();
		character->ClientSetIgnoreInput(false);
	}
}

void ATDMGameModeBase::StartReplays() {
	TArray<AActor*> foundActors;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), AReplayControlledFPSCharacter::StaticClass(), foundActors);
	for(auto& item : foundActors)
		Cast<AReplayControlledFPSCharacter>(item)->StartReplay();
}

AActor* ATDMGameModeBase::FindSpawnForPlayer(AInflectionPointPlayerController * playerController, int round) {
	if(round == 0)
		return FindPlayerStart(playerController);
	return FindPlayerStart(playerController, GetSpawnTag(playerController, round));
}

FString ATDMGameModeBase::GetSpawnTag(AInflectionPointPlayerController*  playerController, int round) {
	std::string spawnTagCString = std::to_string(playerController->Team) +
		playerController->PlayerStartGroup +
		std::to_string(round);
	FString spawnTag(spawnTagCString.c_str());
	return spawnTag;
}

void ATDMGameModeBase::AssignTeamsAndPlayerStartGroups() {
	UWorld* world = GetWorld();

	for(auto iterator = world->GetPlayerControllerIterator(); iterator; ++iterator) {
		AInflectionPointPlayerController* controller = (AInflectionPointPlayerController*)UGameplayStatics::GetPlayerController(world, iterator.GetIndex());
		controller->Team = iterator.GetIndex() % 2 + 1;
		controller->PlayerStartGroup = 'A';
	}
}

void ATDMGameModeBase::SaveRecordingsFromRemainingPlayers() {
	for(FConstPlayerControllerIterator Iterator = GetWorld()->GetPlayerControllerIterator(); Iterator; ++Iterator) {
		auto playerController = UGameplayStatics::GetPlayerController(GetWorld(), Iterator.GetIndex());
		auto ipPlayerController = Cast<AInflectionPointPlayerController>(playerController);
		if(IsPlayerAlive(ipPlayerController))
			SavePlayerRecordings(ipPlayerController);
	}
}

void ATDMGameModeBase::SavePlayerRecordings(AInflectionPointPlayerController * playerController) {
	auto pawn = playerController->GetPawn();
	if(AssertNotNull(pawn, GetWorld(), __FILE__, __LINE__)) {
		auto playerStateRecorder = pawn->FindComponentByClass<UPlayerStateRecorder>();
		AssertNotNull(playerStateRecorder, GetWorld(), __FILE__, __LINE__);
		if(!PlayerRecordings.Contains(playerController)) {
			TMap<int, TArray<FRecordedPlayerState>> map;
			PlayerRecordings.Add(playerController, map);
		}
		PlayerRecordings[playerController].Add(GetGameState()->CurrentRound, playerStateRecorder->RecordedPlayerStates);
	}
}

void ATDMGameModeBase::ClearMap() {
	DestroyAllActors(AReplayControlledFPSCharacter::StaticClass());
	DestroyAllActors(APlayerControlledFPSCharacter::StaticClass());
	DestroyAllActors(AInflectionPointProjectile::StaticClass());
}

void ATDMGameModeBase::DestroyAllActors(TSubclassOf<AActor> actorClass) {
	TArray<AActor*> foundActors;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), actorClass, foundActors);
	for(auto& item : foundActors)
		item->Destroy();
}