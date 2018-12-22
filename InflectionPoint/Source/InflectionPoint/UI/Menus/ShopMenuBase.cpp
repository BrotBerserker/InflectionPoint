// Fill out your copyright notice in the Description page of Project Settings.

#include "InflectionPoint.h"
#include "Gameplay/Shop/BaseShopItem.h"
#include "Gamemodes/TDMPlayerStateBase.h" 
#include "Gameplay/Characters/PlayerCharacterBase.h"
#include "ShopMenuBase.h"

void UShopMenuBase::SetVisibility(ESlateVisibility InVisibility) {
	if(InVisibility == ESlateVisibility::Visible) {
		UpdateShopState(); // automatc sync when visibility changes
	}
	Super::SetVisibility(InVisibility);
}

void UShopMenuBase::UpdateShopState() {
	auto localPlayerState = Cast<ATDMPlayerStateBase>(GetWorld()->GetFirstPlayerController()->PlayerState);
	localPlayerState->ForceNetUpdate();
	InflectionPoints = localPlayerState->IPPoints;
	PurchasedShopItems = localPlayerState->PurchasedShopItems;
	EquippedShopItems.Reset();
	for(auto& item : localPlayerState->EquippedShopItems)
		EquippedShopItems.Add(item.Slot, item.ShopItemClass);
}

void UShopMenuBase::PurchaseShopItem(UBaseShopItem* item) {
	if(!IsShopItemAffordable(item))
		return;
	auto localController = Cast<APlayerControllerBase>(GetWorld()->GetFirstPlayerController());
	AssertNotNull(localController, GetWorld(), __FILE__, __LINE__);
	InflectionPoints -= item->IPPrice;
	PurchasedShopItems.Add(item->GetClass());
	localController->ServerPurchaseShopItem(item->GetClass());
}

bool UShopMenuBase::IsShopItemAffordable(UBaseShopItem* item) {
	return InflectionPoints >= item->IPPrice;
}

bool UShopMenuBase::IsShopItemPurchased(UBaseShopItem* item) {
	return PurchasedShopItems.Contains(item->GetClass());
}

UBaseShopItem* UShopMenuBase::GetEquippedItem(EInventorySlotPosition inventorySlot) {
	if(!EquippedShopItems.Contains(inventorySlot))
		return nullptr;
	return EquippedShopItems[inventorySlot].GetDefaultObject();
}

UBaseShopItem* UShopMenuBase::GetDefaultItem(EInventorySlotPosition inventorySlot) {
	if(DefaultShopItems.Contains(inventorySlot))
		return DefaultShopItems[inventorySlot].GetDefaultObject();
	return nullptr;
}

bool UShopMenuBase::IsDefaultItemActive(EInventorySlotPosition inventorySlot) {
	if(!DefaultShopItems.Contains(inventorySlot) || EquippedShopItems.Contains(inventorySlot))
		return false;
	auto defaultItemClass = DefaultShopItems[inventorySlot];
	for(auto& item : EquippedShopItems) {
		if(item.Value == defaultItemClass)
			return false;
	}
	return true;
}

void UShopMenuBase::EquipItem(EInventorySlotPosition inventorySlot, UBaseShopItem* item) {
	UnequipItemFromSlot(inventorySlot);
	if(!item)
		return;

	for(auto& slot : EquippedShopItems) {
		if(slot.Value == item->GetClass())
			UnequipItemFromSlot(slot.Key);
	}

	auto localController = Cast<APlayerControllerBase>(GetWorld()->GetFirstPlayerController());
	AssertNotNull(localController, GetWorld(), __FILE__, __LINE__);
	EquippedShopItems.Add(inventorySlot, item->GetClass());
	localController->ServerEquipShopItem(inventorySlot, item->GetClass());
}

void UShopMenuBase::UnequipItemFromSlot(EInventorySlotPosition slot) {
	if(!EquippedShopItems.Contains(slot))
		return;
	auto localController = Cast<APlayerControllerBase>(GetWorld()->GetFirstPlayerController());
	AssertNotNull(localController, GetWorld(), __FILE__, __LINE__);
	EquippedShopItems.Remove(slot);
	localController->ServerUnequipShopItemFromSlot(slot);
}