// Copyright 1998-2016 Epic Games, Inc. All Rights Reserved.
#pragma once
#include "GameFramework/GameModeBase.h"
#include "Gameplay/Recording/PlayerStateRecorder.h"
#include "Gameplay/Controllers/PlayerControllerBase.h" 
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
		void UpdateCountdown(TArray<AActor*> controllers, int number);

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
	
	virtual void PostLogin(APlayerController * NewPlayer) override;

private:
	TMap<APlayerController*, TMap<int, TArray<FRecordedPlayerState>>> PlayerRecordings;

private:
	void AssignTeamsAndPlayerStartGroups();

	bool IsWinnerFound();
	TArray<int> GetTeamsAlive();
	bool IsPlayerAlive(APlayerControllerBase* playerController);

	void SpawnPlayersAndReplays();
	FString GetSpawnTag(APlayerControllerBase*  playerController, int round);
	AActor* FindSpawnForPlayer(APlayerControllerBase* playerController, int round);
	void SpawnAndPrepareReplay(APlayerControllerBase* controller, int round);
	void SpawnAndPossessPlayer(APlayerControllerBase* playerController);

	template <typename CharacterType>
	CharacterType* SpawnCharacter(UClass* spawnClass, APlayerControllerBase * playerController, AActor* playerStart);

	void SaveRecordingsFromRemainingPlayers();
	void SavePlayerRecordings(APlayerControllerBase * playerController);

	void ClearMap();
	void DestroyAllActors(TSubclassOf<AActor> actorClass);

	void WriteKillToPlayerStates(AController * KilledPlayer, AController* KillingPlayer);
	void SendKillInfoToPlayers(AController * KilledPlayer, AController* KillingPlayer, AActor* DamageCauser);
	void SendRoundStartedToPlayers(int Round);
	void ResetPlayerScores();
};



