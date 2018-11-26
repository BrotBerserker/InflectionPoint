// Fill out your copyright notice in the Description page of Project Settings.

#include "InflectionPoint.h"
#include "Gameplay/Damage/MortalityProvider.h"
#include "Gameplay/Controllers/PlayerControllerBase.h"
#include "Gameplay/Characters/BaseCharacter.h"
#include "Gamemodes/TDMPlayerStateBase.h"
#include "InflectionPointCheatManager.h"


void UInflectionPointCheatManager::InitCheatManager() {
	// Any initialize stuff You need.
}

void UInflectionPointCheatManager::EnableCharacterDebugArrows(bool enable) {
	IsCharacterDebugArrowsEnabled = enable;
}

void UInflectionPointCheatManager::EnableDebugProjectileLineTrace(bool enable) {
	IsDebugProjectileLineTraceEnabled = enable;
}

void UInflectionPointCheatManager::EnableLineTracing(bool enable) {
	IsLineTracingEnabled = enable;
}

void UInflectionPointCheatManager::EnableAllReplayDebugging(bool enable) {
	EnableCharacterDebugArrows(enable);
	EnableDebugProjectileLineTrace(enable);
	EnableLineTracing(enable);
}

void UInflectionPointCheatManager::EnableHUD(bool enable) {
	IsHUDEnabled = enable;
}


void UInflectionPointCheatManager::OpenShop() {
	auto controller = Cast<APlayerControllerBase>(GetWorld()->GetFirstPlayerController());
	auto playerState = Cast<ATDMPlayerStateBase>(controller->PlayerState);
	playerState->IPPoints = 99;
	controller->ClientShowShop(false);
}

void UInflectionPointCheatManager::CloseShop() {
	auto controller = Cast<APlayerControllerBase>(GetWorld()->GetFirstPlayerController());
	auto playerState = Cast<ATDMPlayerStateBase>(controller->PlayerState);
	auto character = Cast<ABaseCharacter>(controller->GetPawn());
	controller->ClientShowPhaseCountdownNumber(0); // to switch to ingame View
	// Equipp Items
	for(int i = 0; i < playerState->EquippedShopItems.Num(); i++) {
		auto item = playerState->EquippedShopItems[i];
		item.ShopItemClass.GetDefaultObject()->ApplyToCharacter(character, item.Slot);
		if(i == 0)
			character->ServerEquipSpecificWeapon(item.Slot);
	}
}

//void UInflectionPointCheatManager::SetHP(int newHP) {
//	auto player = GetWorld()->GetFirstPlayerController()->GetCharacter();
//	if(player)
//		player->FindComponentByClass<UMortalityProvider>()->CurrentHealth = newHP;
//}