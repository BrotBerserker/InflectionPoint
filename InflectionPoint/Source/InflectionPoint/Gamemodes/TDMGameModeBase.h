// Copyright 1998-2016 Epic Games, Inc. All Rights Reserved.
#pragma once
#include "GameFramework/GameModeBase.h"
#include "Gameplay/Recording/PlayerStateRecorder.h"
#include "Gameplay/Controllers/PlayerControllerBase.h" 
#include "Gamemodes/TDMGameStateBase.h" 
#include "Gamemodes/TDMScoreHandler.h"
#include "TDMGameModeBase.generated.h"

UCLASS(minimalapi)
class ATDMGameModeBase : public AGameModeBase {
	GENERATED_BODY()

public:
	/* ---------------------- */
	/*   Blueprint functions  */
	/* ---------------------- */

	/** Updates the max number of players using the given session */
	UFUNCTION(BlueprintCallable, Category = "InflectionPoint|GameMode")
		void UpdateMaxPlayers(FName SessioName);

	/** Updates the current number of players using the given session */
	UFUNCTION(BlueprintCallable, Category = "InflectionPoint|GameMode")
		void UpdateCurrentPlayers(FName SessionName);

	/** Tells the game mode that a character has been killed by another player or by himself */
	UFUNCTION(BlueprintCallable, Category = "InflectionPoint|GameMode")
		void CharacterDied(AController * KilledPlayer, AController* KillingPlayer, AActor* DamageCauser);

public:
	/* --------------- */
	/*    Functions    */
	/* --------------- */

	/** Constructor */
	ATDMGameModeBase();

	/** Starts the match by starting round 1 */
	UFUNCTION()
		void StartMatch();

	/** Saves the current round's replays and starts the next round */
	UFUNCTION()
		void EndCurrentRound();

	/** Respawns players and replays, start the countdown */
	UFUNCTION()
		void StartNextRound();

	/** Starts the countdown at the beginning of a new round */
	UFUNCTION()
		void StartCountdown();

	/** Informs all players about the next countdown number */
	UFUNCTION()
		void UpdateCountdown(TArray<AActor*> controllers, int number);

	/** Switches to a cinematic camera at the beginning of a new round */
	UFUNCTION()
		void StartSpawnCinematics();

	/** Starts all spawned replays */
	UFUNCTION()
		void StartReplays();

	/** Called when a player connects to the server, starts the match when enouhg players have joined */
	virtual void PostLogin(APlayerController * NewPlayer) override;

	virtual void PreLogin(const FString & Options, const FString & Address, const FUniqueNetIdRepl & UniqueId, FString & ErrorMessage) override;

	/** Returns the GameState as TDMGameStateBase */
	FORCEINLINE class ATDMGameStateBase* GetGameState() const { return (ATDMGameStateBase*)GameState; };

public:
	/* ---------------------- */
	/*    Editor Settings     */
	/* ---------------------- */

	/** Countdown duration in seconds */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		int CountDownDuration = 3;

	/** Seconds to wait before starting the match after enough players have joined */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		float MatchStartDelay = 1.0f;

	/** Seconds to wait before a round is ended after the winner has been decided */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		float RoundEndDelay = 2.0f;

	/** Character to use as Replays */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Debug)
		class UClass* ReplayCharacter;


	/** MaxPlayers is set to this value when playing in editor or offline */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Debug)
		int OfflineMaxPlayers = 2;

public:
	/* ---------------------- */
	/*    Public variables    */
	/* ---------------------- */

	/** Max number of connected players and number of players required to start a match */
	UPROPERTY(BlueprintReadWrite)
		int MaxPlayers = OfflineMaxPlayers;

	/** Current number of connected players */
	UPROPERTY(BlueprintReadWrite)
		int NumPlayers = 0;

	UPROPERTY(BlueprintReadWrite)
		UTDMScoreHandler* ScoreHandler;


private:
	TMap<APlayerController*, TMap<int, TArray<FRecordedPlayerState>>> PlayerRecordings;

private:

	/** Check current round's status */
	bool IsWinnerFound();
	TArray<int> GetTeamsAlive();
	bool IsPlayerAlive(APlayerControllerBase* playerController);

	/** Spawning */
	void AssignTeamsAndPlayerStartGroups();
	void SpawnPlayersAndReplays();
	FString GetSpawnTag(APlayerControllerBase*  playerController, int round);
	AActor* FindSpawnForPlayer(APlayerControllerBase* playerController, int round);
	int CountSpawnPoints();
	void SpawnAndPrepareReplay(APlayerControllerBase* controller, int round);
	void SpawnAndPossessPlayer(APlayerControllerBase* playerController);

	template <typename CharacterType>
	CharacterType* SpawnCharacter(UClass* spawnClass, APlayerControllerBase * playerController, AActor* playerStart);

	/** Start/end rounds */
	void ResetPlayerScores();

	void SaveRecordingsFromRemainingPlayers();
	void SavePlayerRecordings(APlayerControllerBase * playerController);

	void ClearMap();
	void DestroyAllActors(TSubclassOf<AActor> actorClass);
	void DestroyAllActorsWithTag(FName tag);

	/** Inform players about kill etc. */
	void WriteKillToPlayerStates(AController * KilledPlayer, AController* KillingPlayer);
	void SendKillInfoToPlayers(AController * KilledPlayer, AController* KillingPlayer, AActor* DamageCauser);
	void SendRoundStartedToPlayers(int Round);
};



