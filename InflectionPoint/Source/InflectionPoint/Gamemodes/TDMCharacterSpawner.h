// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "TDMCharacterSpawner.generated.h"


UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class INFLECTIONPOINT_API UTDMCharacterSpawner : public UActorComponent {
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	UTDMCharacterSpawner();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:
	/** Characters to use for Players */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Characters)
		TArray<class UClass*> PlayerCharacters;

	/** Character to use as Replays */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Characters)
		TArray<class UClass*> ReplayCharacters;

public:
	/** Returns the number of spawn points on the current map */
	int GetSpawnPointCount();

	/** Assigns each player a team and a start group which is used to determine the spawn position */
	void AssignTeamsAndPlayerStartGroups();

	/** Spawns all players and replays for the given round */
	void SpawnPlayersAndReplays(int CurrentRound, TMap<APlayerController*, TMap<int, TArray<FRecordedPlayerState>>> PlayerRecordings);

	/** Spawns a player for the given round and lets the given controller possess it */
	void SpawnAndPossessPlayer(APlayerControllerBase* playerController, int CurrentRound);

	/** Spawns a replay for the given round and initializes it using the given controller and replay data */
	void SpawnAndPrepareReplay(APlayerControllerBase* controller, int round, TMap<APlayerController*, TMap<int, TArray<FRecordedPlayerState>>> PlayerRecordings);

private:
	class ATDMGameModeBase* gameMode;

private:
	FString GetSpawnTag(APlayerControllerBase*  playerController, int round);
	AActor* FindSpawnForPlayer(APlayerControllerBase* playerController, int round);
	int GetTeam(APlayerControllerBase* playerController);

	template <typename CharacterType>
	CharacterType* SpawnCharacter(UClass* spawnClass, APlayerControllerBase * playerController, AActor* playerStart);
};