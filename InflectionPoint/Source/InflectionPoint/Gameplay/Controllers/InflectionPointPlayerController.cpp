// Fill out your copyright notice in the Description page of Project Settings.

#include "InflectionPoint.h"
#include "DebugTools/InflectionPointCheatManager.h"
#include "InflectionPointPlayerController.h"

AInflectionPointPlayerController::AInflectionPointPlayerController(const FObjectInitializer& ObjectInitializer) :
	Super(ObjectInitializer) {
	CheatClass = UInflectionPointCheatManager::StaticClass();
}


