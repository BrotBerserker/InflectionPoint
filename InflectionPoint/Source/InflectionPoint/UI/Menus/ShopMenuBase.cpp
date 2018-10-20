// Fill out your copyright notice in the Description page of Project Settings.

#include "InflectionPoint.h"
#include "Gameplay/Shop/BaseShopItem.h"
#include "Gamemodes/TDMPlayerStateBase.h" 
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
}

void UShopMenuBase::PurchaseShopItem(UBaseShopItem* item) {
	if(!IsShopItemAffordable(item))
		return;
	auto localPlayerState = Cast<ATDMPlayerStateBase>(GetWorld()->GetFirstPlayerController()->PlayerState);
	AssertNotNull(localPlayerState, GetWorld(), __FILE__, __LINE__);
	InflectionPoints -= item->IPPrice;
	PurchasedShopItems.Add(item->GetClass());
	localPlayerState->ServerPurchaseShopItem(item->GetClass());
}

bool UShopMenuBase::IsShopItemAffordable(UBaseShopItem* item) {
	return InflectionPoints >= item->IPPrice;
}

bool UShopMenuBase::IsShopItemPurchased(UBaseShopItem* item) {
	return PurchasedShopItems.Contains(item->GetClass());
}

UBaseShopItem* UShopMenuBase::GetEquippedItem(FString slotName) {
	if(!EquippedItems.Contains(slotName))
		return nullptr;
	return EquippedItems[slotName].GetDefaultObject();
}

void UShopMenuBase::EquippItem(FString slotName, UBaseShopItem* item) {
	if(!item) {
		UnequippItemFromSlot(slotName);
		return;
	}
	auto newEquippMap = TMap<FString, TSubclassOf<class UBaseShopItem>>();
	for(auto& slot : EquippedItems) {
		if(slot.Key != slotName && slot.Value != item->GetClass())
			newEquippMap.Add(slot.Key, slot.Value);
	}
	newEquippMap.Add(slotName, item->GetClass());
	EquippedItems = newEquippMap;
}

void UShopMenuBase::UnequippItemFromSlot(FString slotName) {
	if(EquippedItems.Contains(slotName))
		EquippedItems.Remove(slotName);
}