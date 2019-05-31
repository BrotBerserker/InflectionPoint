// Copyright 1998-2016 Epic Games, Inc. All Rights Reserved.
#pragma once
#include "GameFramework/GameModeBase.h"
#include "Gameplay/Recording/PlayerStateRecorder.h"
#include "Gameplay/Controllers/PlayerControllerBase.h" 
#include "Gamemodes/TDMGameStateBase.h" 
#include "Gamemodes/TDMScoreHandler.h"
#include "Gamemodes/TDMCharacterSpawner.h"
#include "Gamemodes/Countdown.h"
#include "TDMGameModeBase.generated.h"


UCLASS(minimalapi)
class ATDMGameModeBase : public AGameModeBase {
	GENERATED_BODY()

public:
	void PostInitializeComponents() override;

public:
	/* ---------------------- */
	/*   Blueprint functions  */
	/* ---------------------- */

	/** Updates the max number of players using the given session */
	UFUNCTION(BlueprintCallable, Category = "InflectionPoint|GameMode")
		void InitializeSettings(FName SessioName);

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

	/** Starts the match by starting phase 1 round 1 */
	UFUNCTION()
		void StartMatch();

	/** Restarts the match by spawning all players and going to phase 0 round 0 */
	UFUNCTION()
		void ReStartMatch();

	/** Saves the current phase's replays and starts the next phase */
	UFUNCTION()
		void EndCurrentPhase();

	/** Respawns players and replays, starts the phase countdown */
	UFUNCTION()
		void PreparePhaseStart();

	UFUNCTION()
		void StartNextPhase();

	/** Starts the replays, ends the match if a player has left during the countdown */
	UFUNCTION()
		void StartPhase();

	/** starts the next round */
	UFUNCTION()
		void StartNextRound();

	/** ends the current round */
	UFUNCTION()
		void EndCurrentRound();

	/** Informs all players about the next match countdown number */
	UFUNCTION()
		void UpdateMatchCountdown(int number);

	/** Informs all players about the next phase countdown number */
	UFUNCTION()
		void UpdatePhaseCountdown(int number);

	/** Informs all players about the next shop countdown number */
	UFUNCTION()
		void UpdateShopCountdown(int number);

	/** Shows the shops for all Clients and gives ShopItems and IPs */
	UFUNCTION()
		void ShowShops();

	/** Switches to a cinematic camera at the beginning of a new phase */
	UFUNCTION()
		void StartSpawnCinematics();

	/** Switches to a different camera to display the matches winner and loser */
	UFUNCTION()
		void StartEndMatchSequence();

	/** Informs all players about the end of match */
	UFUNCTION()
		void NotifyControllersOfEndMatch(int winnerTeam);

	/** Informs all players about the end of round */
	UFUNCTION()
		void NotifyControllersOfEndRound(int winnerTeam);

	/** Starts all spawned replays */
	UFUNCTION()
		void StartReplays();

	/** Called when a player connects to the server, starts the match when enouhg players have joined */
	virtual void PostLogin(APlayerController * NewPlayer) override;

	virtual void PreLogin(const FString & Options, const FString & Address, const FUniqueNetIdRepl & UniqueId, FString & ErrorMessage) override;

	virtual void Logout(AController* Exiting) override;

	/** Returns the GameState as TDMGameStateBase */
	FORCEINLINE class ATDMGameStateBase* GetGameState() const { return Cast<ATDMGameStateBase>(GetWorld()->GetGameState()); };

public:
	/* ---------------------- */
	/*    Editor Settings     */
	/* ---------------------- */

	/** Duration of the phase start countdown */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		int PhaseStartDelay = 3;

	/** Seconds to wait before starting the match after enough players have joined */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		float MatchStartDelay = 10.0f;

	/** Seconds to wait before closing the shop and starting the phase */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		float ShopTime = 10.0f;

	/** Seconds to wait before restarting the match after the last one has ended */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		float MatchReStartDelay = 20.0f;

	/** Seconds to wait before a phase is ended after the winner has been decided */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		float PhaseEndDelay = 2.0f;

	/** Seconds to wait before a phase is ended after the winner has been decided */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		float RoundEndDelay = 3.0f;

	/** MaxPlayers is set to this value when playing in editor or offline */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Debug)
		int OfflineMaxPlayers = 2;

	/** MaxRoundNum is set to this value when playing in editor or offline */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Debug)
		int OfflineMaxRoundNum = 3;

	/** MaxRoundNum is set to this value when playing in editor or offline */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Debug)
		bool IsPIEQuickPlayModeEnabled = true;

public:
	/* ---------------------- */
	/*    Public variables    */
	/* ---------------------- */

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadWrite)
		UTDMScoreHandler* ScoreHandler;

	UPROPERTY(VisibleDefaultsOnly)
		class UTDMCharacterSpawner* CharacterSpawner;

	UPROPERTY(VisibleDefaultsOnly)
		class UCountdown* MatchStartCountdown;

	UPROPERTY(VisibleDefaultsOnly)
		class UCountdown* PhaseStartCountdown;

	UPROPERTY(VisibleDefaultsOnly)
		class UCountdown* ShopCountdown;

private:
	// thai ming
	bool isPlayingEndMatchSequence = false;

private:
	/** Check current phase's status */
	bool IsPhaseWinnerFound(AController* controllerToIgnore = nullptr);
	bool IsMatchWinnerFound();
	TArray<int> GetTeamsAlive(AController* controllerToIgnore = nullptr);
	bool IsPlayerAlive(APlayerControllerBase* playerController);

	/** Start/end phases */
	void SaveRecordingsFromRemainingPlayers();
	void SavePlayerRecordings(APlayerControllerBase * playerController);

	void ResetLevel();

	/** Inform players about kill etc. */
	void SendKillInfoToPlayers(AController * KilledPlayer, AController* KillingPlayer, AActor* DamageCauser);
	void SendPhaseStartedToPlayers(int Phase);

	APlayerController* GetAnyPlayerControllerInTeam(int team);

	void EnablePIEQuickPlayMode();
};