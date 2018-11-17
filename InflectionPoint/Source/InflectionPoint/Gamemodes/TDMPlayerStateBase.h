// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "Gameplay/Shop/BaseShopItem.h"
#include "Gameplay/Weapons/WeaponInventory.h"
#include "TDMPlayerStateBase.generated.h"

USTRUCT(BlueprintType)
struct FTDMEquipSlot {
	GENERATED_BODY()

		UPROPERTY(EditAnywhere, BlueprintReadonly)
		EInventorySlotPosition Slot;

	UPROPERTY(EditAnywhere, BlueprintReadonly)
		TSubclassOf<class UBaseShopItem> ShopItemClass;

	bool operator==(const FTDMEquipSlot& str) {
		return Slot == str.Slot;
	}

	FTDMEquipSlot() {}

	FTDMEquipSlot(EInventorySlotPosition slot, TSubclassOf<class UBaseShopItem> shopItem) {
		Slot = slot;
		ShopItemClass = shopItem;
	}
};

/**
 * Seeeehr geil
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
		int IPPoints = 1;

	UPROPERTY(Replicated, VisibleAnywhere, BlueprintReadWrite)
		TArray<TSubclassOf<class UBaseShopItem>> PurchasedShopItems;

	UPROPERTY(Replicated, VisibleAnywhere, BlueprintReadWrite)
		TArray<FTDMEquipSlot> EquippedShopItems; // because TMap has no repilcate

	UPROPERTY(Replicated, BlueprintReadWrite)
		FString ReplicatedPlayerName;

public:

	virtual void SetPlayerName(const FString& S) override;

	virtual FString GetPlayerNameCustom() const override;
};