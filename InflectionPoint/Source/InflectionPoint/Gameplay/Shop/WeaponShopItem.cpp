// Fill out your copyright notice in the Description page of Project Settings.

#include "InflectionPoint.h"
#include "Gameplay/Characters/BaseCharacter.h"
#include "Gameplay/Weapons/WeaponInventory.h"
#include "WeaponShopItem.h"

UWeaponShopItem::UWeaponShopItem() {
	ShopItemType = EShopItemType::Weapon;
}

void UWeaponShopItem::ApplyToCharacter(ABaseCharacter* character, EInventorySlotPosition slot) {
	AssertNotNull(character, GetWorld(), __FILE__, __LINE__);
	AssertNotNull(character->WeaponInventory, GetWorld(), __FILE__, __LINE__);
	character->WeaponInventory->SetWeaponAtPosition(slot, WeaponClass);
}