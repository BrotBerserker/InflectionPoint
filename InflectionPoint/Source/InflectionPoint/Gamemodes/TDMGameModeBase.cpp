// Copyright 1998-2016 Epic Games, Inc. All Rights Reserved.

#include "InflectionPoint.h"
#include "TDMGameModeBase.h"
#include <string>
#include "UI/HUD/InflectionPointHUD.h"
#include "Gameplay/Characters/BaseCharacter.h"
#include "Gameplay/Damage/MortalityProvider.h"
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
}

void ATDMGameModeBase::UpdateMaxPlayers(FName SessioName) {
	IOnlineSessionPtr session = IOnlineSubsystem::Get()->GetSessionInterface();
	FOnlineSessionSettings* sessionSettings = session->GetSessionSettings(SessioName);
	if(sessionSettings) {
		MaxPlayers = sessionSettings->NumPublicConnections;
	} else {
		UE_LOG(LogTemp, Warning, TEXT("Warning: No session settings could be found, setting MaxPlayers to 2."));
		MaxPlayers = 2;
	}
}

void ATDMGameModeBase::StartMatch() {
	AssignTeamsAndPlayerStartGroups();
	StartNextRound();
}

void ATDMGameModeBase::StartNextRound() {
	// Save replays from players that are stil alive
	SaveRecordingsFromRemainingPlayers();
	CurrentRound++;
	SpawnPlayersAndReplays();
}

void ATDMGameModeBase::PlayerDied(AInflectionPointPlayerController * playerController) {
	SavePlayerRecordings(playerController);

	// Destroy PLayer
	if(AssertNotNull(playerController->GetPawn(), GetWorld(), __FILE__, __LINE__))
		playerController->GetPawn()->SetLifeSpan(.00001);

	if(CurrentRound == 0) {
		SpawnPlayer(playerController);
	} else if(IsRoundFinished()) {
		StartNextRound();
	} else {
		// TODO: Set Player as spectator
	}
}

bool ATDMGameModeBase::IsRoundFinished() {
	return true;
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
	if(pawn) {
		auto mortalityProvider = pawn->FindComponentByClass<UMortalityProvider>();
		if(mortalityProvider && mortalityProvider->CurrentHealth > 0)
			return true;
	}
	return false;
}

void ATDMGameModeBase::SpawnPlayer(AInflectionPointPlayerController * playerController) {
	AActor* playerStart = FindSpawnForPlayer(playerController, CurrentRound);
	AssertNotNull(playerStart, GetWorld(), __FILE__, __LINE__);

	APawn* pawn = playerController->GetPawn();
	if(pawn) {
		pawn->SetLifeSpan(.0001);
	}

	FVector loc = playerStart->GetTransform().GetLocation();
	FRotator rot = FRotator(playerStart->GetTransform().GetRotation());

	FActorSpawnParameters ActorSpawnParams;
	ActorSpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	APlayerControlledFPSCharacter* newCharacter = GetWorld()->SpawnActor<APlayerControlledFPSCharacter>(DefaultPawnClass.Get(), loc, rot, ActorSpawnParams);

	playerController->Possess(newCharacter);
}

// TODO: rename
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


void ATDMGameModeBase::SpawnPlayersAndReplays() {
	for(FConstPlayerControllerIterator Iterator = GetWorld()->GetPlayerControllerIterator(); Iterator; ++Iterator) {
		auto playerController = UGameplayStatics::GetPlayerController(GetWorld(), Iterator.GetIndex());
		auto ipPlayerController = Cast<AInflectionPointPlayerController>(playerController);
		SpawnPlayer(ipPlayerController);
		for(int i = 1; i < CurrentRound; i++)
			SpawnReplay(ipPlayerController, i);
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
		PlayerRecordings[playerController].Add(CurrentRound, playerStateRecorder->RecordedPlayerStates);
	}
}

void ATDMGameModeBase::SpawnReplay(AInflectionPointPlayerController* controller, int round) {
	AssertTrue(PlayerRecordings.Contains(controller), GetWorld(), __FILE__, __LINE__, "Could not find replay");
	auto spawn = FindSpawnForPlayer(controller, round);
	AssertNotNull(spawn, GetWorld(), __FILE__, __LINE__, "No spawn found");
	FVector loc = spawn->GetTransform().GetLocation();
	FRotator rot = FRotator(spawn->GetTransform().GetRotation());

	FActorSpawnParameters spawnParams;
	spawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

	// Spawn ReplayCharacter
	AReplayControlledFPSCharacter* newPlayer = GetWorld()->SpawnActor<AReplayControlledFPSCharacter>(ReplayCharacter, loc, rot, spawnParams);
	if(!AssertNotNull(newPlayer, GetWorld(), __FILE__, __LINE__, "Could not spawn replay character!")) {
		return;
	}

	AssertTrue(PlayerRecordings[controller].Contains(round), GetWorld(), __FILE__, __LINE__, "Could not find replay");

	// Start Replay on spawned ReplayCharacter
	if(PlayerRecordings[controller].Contains(round))
		newPlayer->StartReplay(PlayerRecordings[controller][round]);
}