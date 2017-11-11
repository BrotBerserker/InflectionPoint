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
		int GetPlayerKills();
	UFUNCTION(BlueprintCallable, Category = "InflectionPoint|Gameplay")
		int GetReplayKills();
	UFUNCTION(BlueprintCallable, Category = "InflectionPoint|Gameplay")
		int GetDeaths();

private:
	UPROPERTY(Replicated)
		int PlayerKills = 0;
	UPROPERTY(Replicated)
		int ReplayKills = 0;
	UPROPERTY(Replicated)
		int Deaths = 0;

};