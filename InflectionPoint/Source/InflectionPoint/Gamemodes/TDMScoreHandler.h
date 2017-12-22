// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "TDMScoreHandler.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class INFLECTIONPOINT_API UTDMScoreHandler : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UTDMScoreHandler();

public:

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "InflectionPoint")
		float ScorePointsForPlayerKill = 100;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "InflectionPoint")
		float ScorePointsForReplayKill = 50;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "InflectionPoint")
		float ScorePointsForDeath = 0;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "InflectionPoint")
		float ScorePointsForTeamKill = -25;

	UFUNCTION()
		float GetKillerScoreChange(AController * KilledPlayer, AController* KillingPlayer);
	
	UFUNCTION()
		float GetKilledScoreChange(AController * KilledPlayer, AController* KillingPlayer);
	
	UFUNCTION()
		void AddKill(AController * KilledPlayer, AController* KillingPlayer);

private:
	UFUNCTION()
		void AddKillToPlayerScore(AController * KilledPlayer, AController* KillingPlayer);
	UFUNCTION()
		void AddKillToPlayerState(AController * KilledPlayer, AController* KillingPlayer);

	UFUNCTION()
		bool IsTeamKill(UCharacterInfoProvider* killedInfo, UCharacterInfoProvider* killerInfo);
};
