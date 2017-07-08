// Copyright 1998-2016 Epic Games, Inc. All Rights Reserved.
#pragma once
#include "GameFramework/GameModeBase.h"
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


public:
	int MaxPlayers;
	int CurrentRound = 0;

};



