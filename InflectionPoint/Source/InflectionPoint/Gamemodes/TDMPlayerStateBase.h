// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "TDMPlayerStateBase.generated.h"

/**
 *
 */
UCLASS()
class INFLECTIONPOINT_API ATDMPlayerStateBase : public APlayerState {
	GENERATED_BODY()

public:
	void GetLifetimeReplicatedProps(TArray< FLifetimeProperty > & OutLifetimeProps) const;

public:

	UPROPERTY(Replicated, BlueprintReadWrite, Category = "InflectionPoint|Gameplay")
		int Team = 0;

	UPROPERTY(Replicated, BlueprintReadWrite, Category = "InflectionPoint|Gameplay")
		FString PlayerStartGroup;
public:

	UFUNCTION(BlueprintCallable, Category = "InflectionPoint|Gameplay")
		void AddPlayerKill();
	UFUNCTION(BlueprintCallable, Category = "InflectionPoint|Gameplay")
		void AddReplayKill();
	UFUNCTION(BlueprintCallable, Category = "InflectionPoint|Gameplay")
		void AddDeath();
	UFUNCTION(BlueprintCallable, Category = "InflectionPoint|Gameplay")
		void AddTeamKill();

	UFUNCTION(BlueprintCallable, Category = "InflectionPoint|Gameplay")
		int GetPlayerKills();
	UFUNCTION(BlueprintCallable, Category = "InflectionPoint|Gameplay")
		int GetReplayKills();
	UFUNCTION(BlueprintCallable, Category = "InflectionPoint|Gameplay")
		int GetDeaths();
	UFUNCTION(BlueprintCallable, Category = "InflectionPoint|Gameplay")
		int GetTeamKills();


	UFUNCTION(BlueprintCallable, Category = "InflectionPoint|Gameplay")
		void ResetScore();


	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "InflectionPoint")
		float ScorePointsForPlayerKill = 100;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "InflectionPoint")
		float ScorePointsForReplayKill = 50;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "InflectionPoint")
		float ScorePointsForDeath = 0;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "InflectionPoint")
		float ScorePointsForTeamKill = 0;


private:
	UPROPERTY(Replicated)
		int PlayerKills = 0;
	UPROPERTY(Replicated)
		int ReplayKills = 0;
	UPROPERTY(Replicated)
		int Deaths = 0;
	UPROPERTY(Replicated)
		int TeamKills = 0;

};