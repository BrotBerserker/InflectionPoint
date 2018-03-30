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
	UPROPERTY(Replicated, BlueprintReadWrite, Category = "InflectionPoint")
		int CurrentRound = 0;

	UPROPERTY(Replicated, BlueprintReadWrite, Category = "InflectionPoint")
		int MaxRoundNum = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		TArray<FColor> TeamColors;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		TArray<FColor> ReplayTeamColors;

public:
	/** Needed for replication */
	void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
};
