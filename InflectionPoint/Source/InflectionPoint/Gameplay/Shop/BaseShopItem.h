// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Gameplay/Weapons/WeaponInventory.h"
#include "BaseShopItem.generated.h"

UENUM(Blueprintable, Meta = (Bitflags))
enum class EShopItemType : uint8 {
	Skill,
	Weapon,
};

/**
 *
 */
UCLASS(Blueprintable)
class INFLECTIONPOINT_API UBaseShopItem : public UObject {
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		FText Name;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		FText Description;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		int IPPrice;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		UTexture2D* Image;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		EShopItemType ShopItemType;

	UFUNCTION(BlueprintCallable)
		bool IsAffordableForLocalPlayer();

	UFUNCTION(BlueprintCallable)
		bool IsAffordableForPlayer(class ATDMPlayerStateBase* playerState);

	virtual void ApplyToCharacter(class ABaseCharacter* character, enum EInventorySlotPosition slot) PURE_VIRTUAL(UBaseShopItem::ApplyToCharacter, ;);

};
