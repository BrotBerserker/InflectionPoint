// Copyright 1998-2016 Epic Games, Inc. All Rights Reserved.
#pragma once
#include "GameFramework/GameModeBase.h"
#include "Gameplay/Recording/PlayerStateRecorder.h"
#include "Gameplay/Controllers/InflectionPointPlayerController.h" 
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

	UFUNCTION(BlueprintCallable, Category = "InflectionPoint|GameMode")
		void SpawnPlayer(AInflectionPointPlayerController* playerController);

	UFUNCTION()
		void ShowCountdownNumber(APlayerControlledFPSCharacter* character, int number);

public:
	UPROPERTY(BlueprintReadWrite)
		int MaxPlayers;

	UPROPERTY(BlueprintReadWrite)
		int NumPlayers = 0;

	UPROPERTY(BlueprintReadWrite)
		int CurrentRound = 0;	

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Debug)
		class UClass* ReplayCharacter;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Debug)
		int Rounds = 3;

private:
	TMap<APlayerController*, TMap<int,TArray<FRecordedPlayerState>>> PlayerRecordings;
private:
	TArray<int> GetTeamsAlive();
	void AssignTeamsAndPlayerStartGroups();
	bool IsRoundFinished();
	AActor* FindSpawnForPlayer(AInflectionPointPlayerController* playerController, int round);
	FString GetSpawnTag(AInflectionPointPlayerController*  playerController, int round);
	void SaveRecordingsFromRemainingPlayers();
	void SpawnPlayersAndReplays();
	void SavePlayerRecordings(AInflectionPointPlayerController * playerController);
	void SpawnReplay(AInflectionPointPlayerController* controller, int round);
	bool IsPlayerAlive(AInflectionPointPlayerController* playerController); 

};



