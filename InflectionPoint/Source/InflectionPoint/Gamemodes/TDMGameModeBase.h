// Copyright 1998-2016 Epic Games, Inc. All Rights Reserved.
#pragma once
#include "GameFramework/GameModeBase.h"
#include "Gameplay/Recording/PlayerStateRecorder.h"
#include "Gameplay/Controllers/InflectionPointPlayerController.h" 
#include "Gamemodes/TDMGameState.h" 
#include "TDMGameModeBase.generated.h"

UCLASS(minimalapi)
class ATDMGameModeBase : public AGameModeBase {
	GENERATED_BODY()

public:
	ATDMGameModeBase();

	UFUNCTION(BlueprintCallable, Category = "InflectionPoint|GameMode")
		void UpdateMaxPlayers(FName SessioName);

	UFUNCTION(BlueprintCallable, Category = "InflectionPoint|GameMode")
		void StartMatch();

	UFUNCTION(BlueprintCallable, Category = "InflectionPoint|GameMode")
		void StartNextRound();

	UFUNCTION(BlueprintCallable, Category = "InflectionPoint|GameMode")
		void PlayerDied(AInflectionPointPlayerController* playerController);

	void StartCountdown(APlayerControlledFPSCharacter * newCharacter);

	UFUNCTION()
		void UpdateCountdown(APlayerControlledFPSCharacter* character, int number);

	UFUNCTION()
		void StartReplays();

public:
	UPROPERTY(BlueprintReadWrite)
		int MaxPlayers;

	UPROPERTY(BlueprintReadWrite)
		int NumPlayers = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		int CountDownDuration = 3;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Debug)
		class UClass* ReplayCharacter;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Debug)
		int OfflineMaxPlayers = 2;

public:
	FORCEINLINE class ATDMGameState* GetGameState() const { return (ATDMGameState*)GameState; };

private:
	TMap<APlayerController*, TMap<int, TArray<FRecordedPlayerState>>> PlayerRecordings;
private:
	void AssignTeamsAndPlayerStartGroups();

	bool IsRoundFinished();
	TArray<int> GetTeamsAlive();
	bool IsPlayerAlive(AInflectionPointPlayerController* playerController);

	void SpawnPlayersAndReplays();
	FString GetSpawnTag(AInflectionPointPlayerController*  playerController, int round);
	AActor* FindSpawnForPlayer(AInflectionPointPlayerController* playerController, int round);
	void SpawnAndPrepareReplay(AInflectionPointPlayerController* controller, int round);
	void SpawnAndPossessPlayer(AInflectionPointPlayerController* playerController);

	template <typename CharacterType>
	CharacterType* SpawnCharacter(UClass* spawnClass, AInflectionPointPlayerController * playerController, AActor* playerStart);

	void SaveRecordingsFromRemainingPlayers();
	void SavePlayerRecordings(AInflectionPointPlayerController * playerController);

	void ClearMap();
	void DestroyAllActors(TSubclassOf<AActor> actorClass);
};



