// Fill out your copyright notice in the Description page of Project Settings.

#include "InflectionPoint.h"
#include "Gameplay/Characters/PlayerCharacterBase.h"
#include "Gameplay/Characters/ReplayCharacterBase.h"
#include "Gameplay/Controllers/PlayerControllerBase.h"
#include "Gameplay/Controllers/AIControllerBase.h"
#include "Gamemodes/TDMGameModeBase.h"
#include "TDMGameModeBase.h"
#include "TDMCharacterSpawner.h"

UTDMCharacterSpawner::UTDMCharacterSpawner() {
	PrimaryComponentTick.bCanEverTick = true;
}

void UTDMCharacterSpawner::BeginPlay() {
	Super::BeginPlay();

	gameMode = Cast<ATDMGameModeBase>(GetOwner());
}

void UTDMCharacterSpawner::SpawnPlayersAndReplays(int CurrentPhase) {	
	DoShitForAllPlayerControllers(GetWorld(), [&](APlayerControllerBase* controller) {
		SpawnAndPossessPlayer(controller, CurrentPhase);
		SpawnAndPrepareReplays(controller, CurrentPhase);
	});
}

void UTDMCharacterSpawner::SpawnAndPossessPlayer(APlayerControllerBase * playerController, int CurrentPhase) {
	auto playerState = Cast<ATDMPlayerStateBase>(playerController->PlayerState);
	AssertNotNull(playerState, GetWorld(), __FILE__, __LINE__, "No spawn found");
	auto spawnPoint = FindSpawnForPlayer(playerController, CurrentPhase);
	AssertNotNull(spawnPoint, GetWorld(), __FILE__, __LINE__, "No spawn found");

	auto character = SpawnCharacter<APlayerCharacterBase>(PlayerCharacters[GetTeam(playerController)], spawnPoint);

	playerController->ClientSetControlRotation(FRotator(spawnPoint->GetTransform().GetRotation()));
	playerController->Possess(character);
	EquipShopItems(character, playerState->EquippedShopItems);
	Cast<ATDMPlayerStateBase>(playerController->PlayerState)->IsAlive = true;
}

void UTDMCharacterSpawner::SpawnAndPrepareReplays(APlayerControllerBase* controller, int CurrentPhase) {
	for(int i = 0; i < CurrentPhase - 1; i++) {
		auto data = controller->RecordedPlayerData[i];
		AssertTrue(data.Phase < CurrentPhase, GetWorld(), __FILE__, __LINE__, "Replay data mismatch");
		SpawnAndPrepareReplay(controller, data);
	}
}

void UTDMCharacterSpawner::SpawnAndPrepareReplay(APlayerControllerBase* playerController, FRecordedPlayerData playerRecordings) {
	auto spawnPoint = FindSpawnForPlayer(playerController, playerRecordings.Phase);
	AssertNotNull(spawnPoint, GetWorld(), __FILE__, __LINE__, "No spawn found");

	auto character = SpawnCharacter<AReplayCharacterBase>(ReplayCharacters[GetTeam(playerController)], spawnPoint);

	character->SetReplayData(playerRecordings.RecordedPlayerStates);
	character->ReplayIndex = playerRecordings.Phase;
	EquipShopItems(character, playerRecordings.EquippedShopItems);
	Cast<AAIControllerBase>(character->GetController())->Initialize(playerController);
}

template <typename CharacterType>
CharacterType* UTDMCharacterSpawner::SpawnCharacter(UClass* spawnClass, AActor* playerStart) {
	FVector loc = playerStart->GetTransform().GetLocation();
	FRotator rot = FRotator(playerStart->GetTransform().GetRotation());

	FActorSpawnParameters ActorSpawnParams;
	ActorSpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	CharacterType* newCharacter = GetWorld()->SpawnActor<CharacterType>(spawnClass, loc, rot, ActorSpawnParams);
	AssertNotNull(newCharacter, GetWorld(), __FILE__, __LINE__, "Could not spawn character!");
	return newCharacter;
}

int UTDMCharacterSpawner::GetTeam(APlayerControllerBase* playerController) {
	auto playerState = Cast<ATDMPlayerStateBase>(playerController->PlayerState);
	AssertNotNull(playerState, GetWorld(), __FILE__, __LINE__);
	return playerState->Team;
}

AActor* UTDMCharacterSpawner::FindSpawnForPlayer(APlayerControllerBase * playerController, int phase) {
	if(phase == 0)
		return gameMode->FindPlayerStart(playerController);
	return gameMode->FindPlayerStart(playerController, GetSpawnTag(playerController, phase));
}

FString UTDMCharacterSpawner::GetSpawnTag(APlayerControllerBase*  playerController, int phase) {
	auto playerState = Cast<ATDMPlayerStateBase>(playerController->PlayerState);
	int teams = gameMode->GetGameState()->TeamCount;//2;
	int playersPerTeam = (gameMode->GetGameState()->MaxPlayers / teams);
	int spawnsPerTeam = GetSpawnPointCount() / teams;
	int spawnsPerPlayer = spawnsPerTeam / playersPerTeam;
	int spawnIndex = playerState->PlayerStartGroup * spawnsPerPlayer + phase;
	FString spawnTag = FString::FromInt(playerState->Team) + "|" + FString::FromInt(spawnIndex);
	return spawnTag;
}

int UTDMCharacterSpawner::GetSpawnPointCount() {
	TArray<AActor*> foundActors;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), APlayerStart::StaticClass(), foundActors);
	return foundActors.Num();
}

void UTDMCharacterSpawner::AssignTeamsAndPlayerStartGroups() {
	UWorld* world = GetWorld();

	for(auto iterator = world->GetPlayerControllerIterator(); iterator; ++iterator) {
		APlayerControllerBase* controller = (APlayerControllerBase*)UGameplayStatics::GetPlayerController(world, iterator.GetIndex());
		ATDMPlayerStateBase* playerState = Cast<ATDMPlayerStateBase>(controller->PlayerState);
		AssertNotNull(playerState, GetWorld(), __FILE__, __LINE__);
		int teams = gameMode->GetGameState()->TeamCount;//2;
		playerState->Team = iterator.GetIndex() % teams + 1;
		playerState->PlayerStartGroup = iterator.GetIndex() / 2; // TODO: solve this for != 2 teams
	}
}

void UTDMCharacterSpawner::SpawnAllPlayersForWarmupRound() {
	for(FConstPlayerControllerIterator Iterator = GetWorld()->GetPlayerControllerIterator(); Iterator; ++Iterator) {
		auto playerController = UGameplayStatics::GetPlayerController(GetWorld(), Iterator.GetIndex());
		APlayerControllerBase* controller = Cast<APlayerControllerBase>(playerController);
		controller->ClientPhaseStarted(0);
		SpawnAndPossessPlayer(controller, 0);
	}
}

void UTDMCharacterSpawner::EquipShopItems(ABaseCharacter* character, TArray<FTDMEquipSlot> equippedItems) {
	for(int i = 0; i < equippedItems.Num(); i++) {
		auto item = equippedItems[i];
		item.ShopItemClass.GetDefaultObject()->ApplyToCharacter(character, item.Slot);
	}
}