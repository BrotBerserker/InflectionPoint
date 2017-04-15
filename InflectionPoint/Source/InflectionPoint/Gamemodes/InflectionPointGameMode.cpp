// Copyright 1998-2016 Epic Games, Inc. All Rights Reserved.

#include "InflectionPoint.h"
#include "InflectionPointGameMode.h"
#include "UI/HUD/InflectionPointHUD.h"
#include "Gameplay/Characters/BaseCharacter.h"

AInflectionPointGameMode::AInflectionPointGameMode()
	: Super() {
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnClassFinder(TEXT("/Game/FirstPersonCPP/Blueprints/Characters/PlayerCharacter"));
	DefaultPawnClass = PlayerPawnClassFinder.Class;

	// use our custom HUD class
	HUDClass = AInflectionPointHUD::StaticClass();
}
