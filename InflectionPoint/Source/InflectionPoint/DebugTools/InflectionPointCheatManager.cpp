// Fill out your copyright notice in the Description page of Project Settings.

#include "InflectionPoint.h"
#include "Gameplay/Damage/MortalityProvider.h"
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

//void UInflectionPointCheatManager::SetHP(int newHP) {
//	auto player = GetWorld()->GetFirstPlayerController()->GetCharacter();
//	if(player)
//		player->FindComponentByClass<UMortalityProvider>()->CurrentHealth = newHP;
//}