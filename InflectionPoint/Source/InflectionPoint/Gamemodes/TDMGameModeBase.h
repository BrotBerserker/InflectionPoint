// Copyright 1998-2016 Epic Games, Inc. All Rights Reserved.
#pragma once
#include "GameFramework/GameModeBase.h"
#include "Gameplay/Recording/PlayerStateRecorder.h"
#include "Gameplay/Controllers/InflectionPointPlayerController.h" 
#include "Gamemodes/TDMGameStateBase.h" 
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

	UFUNCTION()
		void EndCurrentRound();

	UFUNCTION()
		void StartNextRound();

	UFUNCTION(BlueprintCallable, Category = "InflectionPoint|GameMode")
		void CharacterDied(AController * KilledPlayer, AController* KillingPlayer, AActor* DamageCauser);

	UFUNCTION()
		void StartCountdown();

	UFUNCTION()
		void StartSpawnCinematics();

	UFUNCTION()
		void UpdateCountdown(TArray<AActor*> characters, int number);

	UFUNCTION()
		void StartReplays();

public:
	UPROPERTY(BlueprintReadWrite)
		int MaxPlayers;

	UPROPERTY(BlueprintReadWrite)
		int NumPlayers = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		int CountDownDuration = 3;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		float RoundEndDelay = 2.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Debug)
		class UClass* ReplayCharacter;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Debug)
		int OfflineMaxPlayers = 2;

public:
	FORCEINLINE class ATDMGameStateBase* GetGameState() const { return (ATDMGameStateBase*)GameState; };

private:
	TMap<APlayerController*, TMap<int, TArray<FRecordedPlayerState>>> PlayerRecordings;

private:
	void AssignTeamsAndPlayerStartGroups();

	bool IsWinnerFound();
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

	void WriteKillToPlayerStates(AController * KilledPlayer, AController* KillingPlayer);
	void SendKillInfoToPlayers(AController * KilledPlayer, AController* KillingPlayer, AActor* DamageCauser);
	void ResetPlayerScores();
};



