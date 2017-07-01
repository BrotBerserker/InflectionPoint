// Copyright 1998-2016 Epic Games, Inc. All Rights Reserved.

#include "InflectionPoint.h"
#include "InflectionPointGameMode.h"
#include "UI/HUD/InflectionPointHUD.h"
#include "Gameplay/Characters/BaseCharacter.h"
#include "Gameplay/Controllers/InflectionPointPlayerController.h"
#include "Gameplay/Characters/PlayerControlledFPSCharacter.h"

AInflectionPointGameMode::AInflectionPointGameMode()
	: Super() {
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnClassFinder(TEXT("/Game/InflectionPoint/Blueprints/Characters/PlayerCharacter"));
	DefaultPawnClass = PlayerPawnClassFinder.Class;
	
	// use our custom HUD class
	HUDClass = AInflectionPointHUD::StaticClass();
	PlayerControllerClass = AInflectionPointPlayerController::StaticClass();
}

void AInflectionPointGameMode::PlayerDied(APlayerController * playerController) {
	AActor* playerStart = FindPlayerStart(playerController);
	AssertNotNull(playerStart, GetWorld(), __FILE__, __LINE__);

	FVector loc = playerStart->GetTransform().GetLocation();
	FRotator rot = FRotator(playerStart->GetTransform().GetRotation());

	FActorSpawnParameters ActorSpawnParams;
	ActorSpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	APlayerControlledFPSCharacter* newCharacter = GetWorld()->SpawnActor<APlayerControlledFPSCharacter>(DefaultPawnClass.Get(), loc, rot, ActorSpawnParams);
	
	playerController->Possess(newCharacter);
}
