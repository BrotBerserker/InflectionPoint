// Fill out your copyright notice in the Description page of Project Settings.

#include "InflectionPoint.h"
#include "Gameplay/Characters/BaseCharacter.h"
#include "Gameplay/Weapons/WeaponInventory.h"
#include "WeaponShopItem.h"


void UWeaponShopItem::ApplyToCharacter(ABaseCharacter* character, EInventorySlot slot) {
	character->WeaponInventory->AddWeapon(slot, WeaponClass);
}