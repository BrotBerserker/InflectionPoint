// Fill out your copyright notice in the Description page of Project Settings.

#include "InflectionPoint.h"
#include "Gameplay/Characters/PlayerCharacterBase.h"
#include "Gameplay/Characters/ReplayCharacterBase.h"
#include "Gameplay/Controllers/PlayerControllerBase.h"
#include "Gameplay/Controllers/AIControllerBase.h"
#include "Gamemodes/TDMGameModeBase.h"
#include "TDMGameModeBase.h"
#include "TDMCharacterSpawner.h"

UTDMCharacterSpawner::UTDMCharacterSpawner() {DebugPrint(__FILE__, __LINE__);
	PrimaryComponentTick.bCanEverTick = true;
DebugPrint(__FILE__, __LINE__);}

void UTDMCharacterSpawner::BeginPlay() {DebugPrint(__FILE__, __LINE__);
	Super::BeginPlay();

	gameMode = Cast<ATDMGameModeBase>(GetOwner());
DebugPrint(__FILE__, __LINE__);}

void UTDMCharacterSpawner::SpawnPlayersAndReplays(int CurrentPhase, TMap<APlayerController*, TArray<FRecordedPlayerData>> PlayerRecordings) {DebugPrint(__FILE__, __LINE__);	
	DoShitForAllPlayerControllers(GetWorld(), [&](APlayerControllerBase* controller) {DebugPrint(__FILE__, __LINE__);
		SpawnAndPossessPlayer(controller, CurrentPhase);
		SpawnAndPrepareReplays(controller, CurrentPhase, PlayerRecordings);
	DebugPrint(__FILE__, __LINE__);});
DebugPrint(__FILE__, __LINE__);}

void UTDMCharacterSpawner::SpawnAndPossessPlayer(APlayerControllerBase * playerController, int CurrentPhase) {DebugPrint(__FILE__, __LINE__);
	auto playerState = Cast<ATDMPlayerStateBase>(playerController->PlayerState);
	AssertNotNull(playerState, GetWorld(), __FILE__, __LINE__, "No spawn found");
	auto spawnPoint = FindSpawnForPlayer(playerController, CurrentPhase);
	AssertNotNull(spawnPoint, GetWorld(), __FILE__, __LINE__, "No spawn found");

	auto character = SpawnCharacter<APlayerCharacterBase>(PlayerCharacters[GetTeam(playerController)], spawnPoint);

	playerController->ClientSetControlRotation(FRotator(spawnPoint->GetTransform().GetRotation()));
	playerController->Possess(character);
	EquipShopItems(character, playerState->EquippedShopItems);
	Cast<ATDMPlayerStateBase>(playerController->PlayerState)->IsAlive = true;
DebugPrint(__FILE__, __LINE__);}

void UTDMCharacterSpawner::SpawnAndPrepareReplays(APlayerControllerBase* controller, int CurrentPhase, TMap<APlayerController*, TArray<FRecordedPlayerData>> PlayerRecordings) {DebugPrint(__FILE__, __LINE__);
	if(PlayerRecordings.Num() == 0)
		return;
	for(int i = 0; i < PlayerRecordings[controller].Num(); i++) {DebugPrint(__FILE__, __LINE__);
		auto data = PlayerRecordings[controller][i];
		AssertTrue(data.Phase < CurrentPhase, GetWorld(), __FILE__, __LINE__, "Replay data mismatch");
		SpawnAndPrepareReplay(controller, data);
	DebugPrint(__FILE__, __LINE__);}
DebugPrint(__FILE__, __LINE__);}

void UTDMCharacterSpawner::SpawnAndPrepareReplay(APlayerControllerBase* playerController, FRecordedPlayerData playerRecordings) {DebugPrint(__FILE__, __LINE__);
	auto spawnPoint = FindSpawnForPlayer(playerController, playerRecordings.Phase);
	AssertNotNull(spawnPoint, GetWorld(), __FILE__, __LINE__, "No spawn found");

	auto character = SpawnCharacter<AReplayCharacterBase>(ReplayCharacters[GetTeam(playerController)], spawnPoint);

	character->SetReplayData(playerRecordings.RecordedPlayerStates);
	character->ReplayIndex = playerRecordings.Phase;
	EquipShopItems(character, playerRecordings.EquippedShopItems);
	Cast<AAIControllerBase>(character->GetController())->Initialize(playerController);
DebugPrint(__FILE__, __LINE__);}

template <typename CharacterType>
CharacterType* UTDMCharacterSpawner::SpawnCharacter(UClass* spawnClass, AActor* playerStart) {DebugPrint(__FILE__, __LINE__);
	FVector loc = playerStart->GetTransform().GetLocation();
	FRotator rot = FRotator(playerStart->GetTransform().GetRotation());

	FActorSpawnParameters ActorSpawnParams;
	ActorSpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	CharacterType* newCharacter = GetWorld()->SpawnActor<CharacterType>(spawnClass, loc, rot, ActorSpawnParams);
	AssertNotNull(newCharacter, GetWorld(), __FILE__, __LINE__, "Could not spawn character!");
	return newCharacter;
DebugPrint(__FILE__, __LINE__);}

int UTDMCharacterSpawner::GetTeam(APlayerControllerBase* playerController) {DebugPrint(__FILE__, __LINE__);
	auto playerState = Cast<ATDMPlayerStateBase>(playerController->PlayerState);
	AssertNotNull(playerState, GetWorld(), __FILE__, __LINE__);
	return playerState->Team;
DebugPrint(__FILE__, __LINE__);}

AActor* UTDMCharacterSpawner::FindSpawnForPlayer(APlayerControllerBase * playerController, int phase) {DebugPrint(__FILE__, __LINE__);
	if(phase == 0)
		return gameMode->FindPlayerStart(playerController);
	return gameMode->FindPlayerStart(playerController, GetSpawnTag(playerController, phase));
DebugPrint(__FILE__, __LINE__);}

FString UTDMCharacterSpawner::GetSpawnTag(APlayerControllerBase*  playerController, int phase) {DebugPrint(__FILE__, __LINE__);
	auto playerState = Cast<ATDMPlayerStateBase>(playerController->PlayerState);
	int teams = gameMode->GetGameState()->TeamCount;//2;
	int playersPerTeam = (gameMode->GetGameState()->MaxPlayers / teams);
	int spawnsPerTeam = GetSpawnPointCount() / teams;
	int spawnsPerPlayer = spawnsPerTeam / playersPerTeam;
	int spawnIndex = playerState->PlayerStartGroup * spawnsPerPlayer + phase;
	FString spawnTag = FString::FromInt(playerState->Team) + "|" + FString::FromInt(spawnIndex);
	return spawnTag;
DebugPrint(__FILE__, __LINE__);}

int UTDMCharacterSpawner::GetSpawnPointCount() {DebugPrint(__FILE__, __LINE__);
	TArray<AActor*> foundActors;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), APlayerStart::StaticClass(), foundActors);
	return foundActors.Num();
DebugPrint(__FILE__, __LINE__);}

void UTDMCharacterSpawner::AssignTeamsAndPlayerStartGroups() {DebugPrint(__FILE__, __LINE__);
	UWorld* world = GetWorld();

	for(auto iterator = world->GetPlayerControllerIterator(); iterator; ++iterator) {DebugPrint(__FILE__, __LINE__);
		APlayerControllerBase* controller = (APlayerControllerBase*)UGameplayStatics::GetPlayerController(world, iterator.GetIndex());
		ATDMPlayerStateBase* playerState = Cast<ATDMPlayerStateBase>(controller->PlayerState);
		AssertNotNull(playerState, GetWorld(), __FILE__, __LINE__);
		int teams = gameMode->GetGameState()->TeamCount;//2;
		playerState->Team = iterator.GetIndex() % teams + 1;
		playerState->PlayerStartGroup = iterator.GetIndex() / 2; // TODO: solve this for != 2 teams
	DebugPrint(__FILE__, __LINE__);}
DebugPrint(__FILE__, __LINE__);}

void UTDMCharacterSpawner::SpawnAllPlayersForWarmupRound() {DebugPrint(__FILE__, __LINE__);
	for(FConstPlayerControllerIterator Iterator = GetWorld()->GetPlayerControllerIterator(); Iterator; ++Iterator) {DebugPrint(__FILE__, __LINE__);
		auto playerController = UGameplayStatics::GetPlayerController(GetWorld(), Iterator.GetIndex());
		APlayerControllerBase* controller = Cast<APlayerControllerBase>(playerController);
		controller->ClientPhaseStarted(0);
		SpawnAndPossessPlayer(controller, 0);
	DebugPrint(__FILE__, __LINE__);}
DebugPrint(__FILE__, __LINE__);}

void UTDMCharacterSpawner::EquipShopItems(ABaseCharacter* character, TArray<FTDMEquipSlot> equippedItems) {DebugPrint(__FILE__, __LINE__);
	for(int i = 0; i < equippedItems.Num(); i++) {DebugPrint(__FILE__, __LINE__);
		auto item = equippedItems[i];
		item.ShopItemClass.GetDefaultObject()->ApplyToCharacter(character, item.Slot);
	DebugPrint(__FILE__, __LINE__);}
DebugPrint(__FILE__, __LINE__);}