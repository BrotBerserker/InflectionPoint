// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Gameplay/Shop/BaseShopItem.h"
#include "WeaponShopItem.generated.h"

/**
 *
 */
UCLASS()
class INFLECTIONPOINT_API UWeaponShopItem : public UBaseShopItem {
	GENERATED_BODY()
public:
	UWeaponShopItem();

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
		TSubclassOf<ABaseWeapon> WeaponClass;

	virtual void ApplyToCharacter(class ABaseCharacter* character, enum EInventorySlot slot) override;
};
