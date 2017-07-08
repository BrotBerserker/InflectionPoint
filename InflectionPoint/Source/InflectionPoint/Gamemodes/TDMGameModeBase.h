// Copyright 1998-2016 Epic Games, Inc. All Rights Reserved.
#pragma once
#include "GameFramework/GameModeBase.h"
#include "Gameplay/Controllers/InflectionPointPlayerController.h" 
#include "TDMGameModeBase.generated.h"

UCLASS(minimalapi)
class ATDMGameModeBase : public AGameModeBase {
	GENERATED_BODY()

public:
	ATDMGameModeBase();

	UFUNCTION(BlueprintCallable, Category = "InflectionPoint|GameMode")
		void PlayerDied(APlayerController* playerController);

	UFUNCTION(BlueprintCallable, Category = "InflectionPoint|GameMode")
		void StartMatch();

	UFUNCTION(BlueprintCallable, Category = "InflectionPoint|GameMode")
		void StartNextRound();

	UFUNCTION(BlueprintCallable, Category = "InflectionPoint|GameMode")
		void SpawnPlayer(APlayerController* playerController);


	UFUNCTION(BlueprintCallable, Category = "InflectionPoint|GameMode")
		AActor* FindSpawnForPlayer(APlayerController* playerController);


	UFUNCTION(BlueprintImplementableEvent, Category = "InflectionPoint")
		void OnRoundFinished(int round);

public:
	int MaxPlayers;
	int CurrentRound = 0;

private:
	bool IsRoundFinished();
	FString GetSpawnTag(AInflectionPointPlayerController*  playerController);

};



