// Fill out your copyright notice in the Description page of Project Settings.

#include "InflectionPoint.h"
#include "InflectionPointCheatManager.h"


void UInflectionPointCheatManager::InitCheatManager() {
	// Any initialize stuff You need.
}

void UInflectionPointCheatManager::PrintTest() {
	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, TEXT("Test"));
}