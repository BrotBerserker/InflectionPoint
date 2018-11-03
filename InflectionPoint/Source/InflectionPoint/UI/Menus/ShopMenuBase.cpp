// Fill out your copyright notice in the Description page of Project Settings.

#include "InflectionPoint.h"
#include "Gameplay/Shop/BaseShopItem.h"
#include "Gamemodes/TDMPlayerStateBase.h" 
#include "Gameplay/Characters/PlayerCharacterBase.h"
#include "ShopMenuBase.h"

void UShopMenuBase::SetVisibility(ESlateVisibility InVisibility) {	
	SyncShopWithPlayerState(); // automatc sync when visibility changes
	OnPreVisibilityChange(InVisibility);
	Super::SetVisibility(InVisibility);
}

void UShopMenuBase::SyncShopWithPlayerState() {
	auto localPlayerState = Cast<ATDMPlayerStateBase>(GetWorld()->GetFirstPlayerController()->PlayerState);
	InflectionPoints = localPlayerState->IPPoints;
	PurchasedShopItems = localPlayerState->PurchasedShopItems;
	EquippedItems.Reset();
	for(auto& item : localPlayerState->EquippedItems)
		EquippedItems.Add(item.Slot, item.ShopItemClass);
	UE_LOG(LogTemp, Warning, TEXT("The value of 'variable' is: %i"), EquippedItems.Num());
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

UBaseShopItem* UShopMenuBase::GetEquippedItem(EInventorySlot inventorySlot) {
	if(!EquippedItems.Contains(inventorySlot))
		return nullptr;
	return EquippedItems[inventorySlot].GetDefaultObject();
}

void UShopMenuBase::EquippItem(EInventorySlot inventorySlot, UBaseShopItem* item) {
	UnequippItemFromSlot(inventorySlot);
	if(!item) 
		return;

	for(auto& slot : EquippedItems) {
		if(slot.Value == item->GetClass())
			UnequippItemFromSlot(slot.Key);
	}
	
	auto localController = Cast<APlayerControllerBase>(GetWorld()->GetFirstPlayerController());
	AssertNotNull(localController, GetWorld(), __FILE__, __LINE__);
	EquippedItems.Add(inventorySlot, item->GetClass());
	localController->ServerEquippShopItem(inventorySlot, item->GetClass());
}

void UShopMenuBase::UnequippItemFromSlot(EInventorySlot slot) {
	if(!EquippedItems.Contains(slot))
		return;
	auto localController = Cast<APlayerControllerBase>(GetWorld()->GetFirstPlayerController());
	AssertNotNull(localController, GetWorld(), __FILE__, __LINE__);
	EquippedItems.Remove(slot);
	localController->ServerUnequippShopItemFromSlot(slot);
}