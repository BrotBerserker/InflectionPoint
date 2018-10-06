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

	ATDMPlayerStateBase();

	void GetLifetimeReplicatedProps(TArray< FLifetimeProperty > & OutLifetimeProps) const;

public:

	UPROPERTY(Replicated, BlueprintReadWrite, Category = "InflectionPoint|Gameplay")
		int Team = 0;

	UPROPERTY(Replicated, BlueprintReadWrite, Category = "InflectionPoint|Gameplay")
		int PlayerStartGroup = 1;
public:

	UFUNCTION(BlueprintCallable, Category = "InflectionPoint|Gameplay")
		void AddScoreToTotalScore();
	UFUNCTION(BlueprintCallable, Category = "InflectionPoint|Gameplay")
		void SetCurrentScoreToTotalScore();
	UFUNCTION(BlueprintCallable, Category = "InflectionPoint|Gameplay")
		void ResetScore();
	UFUNCTION(BlueprintCallable, Category = "InflectionPoint|Gameplay")
		void ResetTotalScore();

	UPROPERTY(Replicated, BlueprintReadWrite)
		float TotalScore = 0;

	UPROPERTY(Replicated, BlueprintReadWrite)
		int PlayerKills = 0;
	UPROPERTY(Replicated, BlueprintReadWrite)
		int TotalPlayerKills = 0;
	UPROPERTY(Replicated, BlueprintReadWrite)
		int ReplayKills = 0;
	UPROPERTY(Replicated, BlueprintReadWrite)
		int TotalReplayKills = 0;
	UPROPERTY(Replicated, BlueprintReadWrite)
		int Deaths = 0;
	UPROPERTY(Replicated, BlueprintReadWrite)
		int TotalDeaths = 0;
	UPROPERTY(Replicated, BlueprintReadWrite)
		int TeamKills = 0;
	UPROPERTY(Replicated, BlueprintReadWrite)
		int TotalTeamKills = 0;
	UPROPERTY(Replicated, BlueprintReadWrite)
		bool IsAlive = 1;

	UPROPERTY(Replicated, BlueprintReadWrite)
		int IPPoints = 3;

	UPROPERTY(Replicated, VisibleAnywhere, BlueprintReadWrite)
		TArray<TSubclassOf<class UBaseShopItem>> PurchasedShopItems;

	UPROPERTY(Replicated, BlueprintReadWrite)
		FString ReplicatedPlayerName;

public:

	virtual void SetPlayerName(const FString& S) override;

	virtual FString GetPlayerNameCustom() const override;

	UFUNCTION(Server, Reliable, BlueprintCallable, WithValidation)
		void ServerPurchaseShopItem(TSubclassOf<class UBaseShopItem> itemClass);
};