// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameState.h"
#include "TDMGameState.generated.h"

/**
 *
 */
UCLASS()
class INFLECTIONPOINT_API ATDMGameState : public AGameStateBase {
	GENERATED_BODY()

public:
	UPROPERTY(Replicated, BlueprintReadWrite, Category = "InflectionPoint")
		int CurrentRound = 0;

public:
	/** Needed for replication */
	void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
};
