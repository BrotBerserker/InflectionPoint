// Fill out your copyright notice in the Description page of Project Settings.

#include "InflectionPoint.h"
#include "Gameplay/Characters/BaseCharacter.h"
#include "Gameplay/Weapons/WeaponInventory.h"
#include "WeaponShopItem.h"

UWeaponShopItem::UWeaponShopItem() {
	ShopItemType = EShopItemType::Weapon;
}

void UWeaponShopItem::ApplyToCharacter(ABaseCharacter* character, EInventorySlotPosition slot) {
	character->WeaponInventory->SetWeaponAtPosition(slot, WeaponClass);
}