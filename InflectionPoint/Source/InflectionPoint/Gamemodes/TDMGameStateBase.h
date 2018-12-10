// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameState.h"
#include "TDMGameStateBase.generated.h"

/**
 *
 */
UCLASS()
class INFLECTIONPOINT_API ATDMGameStateBase : public AGameStateBase {
	GENERATED_BODY()

public:
	/** Max number of connected players and number of players required to start a match */
	UPROPERTY(Replicated, BlueprintReadWrite, Category = "InflectionPoint")
		int MaxPlayers = 2;

	/** Current number of connected players */
	UPROPERTY(Replicated, BlueprintReadWrite, Category = "InflectionPoint")
		int NumPlayers = 0;

	// Number of Teams there are
	UPROPERTY(Replicated, BlueprintReadWrite, Category = "InflectionPoint")
		int TeamCount = 2;

	UPROPERTY(Replicated, BlueprintReadWrite, Category = "InflectionPoint")
		int CurrentRound = 0;

	UPROPERTY(Replicated, BlueprintReadWrite, Category = "InflectionPoint")
		int MaxRoundNum = 3;

	UPROPERTY(Replicated, BlueprintReadWrite, Category = "InflectionPoint")
		int CurrentPhase = 0;

	UPROPERTY(Replicated, BlueprintReadWrite, Category = "InflectionPoint")
		int MaxPhaseNum = 1;

	UPROPERTY(Replicated, BlueprintReadWrite, Category = "InflectionPoint")
		TArray<int> TeamWins;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		TArray<FColor> TeamColors;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		TArray<FColor> ReplayTeamColors;

public:
	/** Needed for replication */
	void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UFUNCTION(BlueprintCallable, Category = "InflectionPoint|Gameplay")
		int GetTeamScore(int team);

	UFUNCTION()
		void PrepareForMatchStart(int SpawnPointCount);

	UFUNCTION()
		void PrepareForRoundStart();

	UFUNCTION()
		void ResetPlayerScores();
	UFUNCTION()
		void ResetTotalPlayerScores();

	void PreparePlayerStatesForRoundStart();
};
