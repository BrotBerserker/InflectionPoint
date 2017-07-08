// Copyright 1998-2016 Epic Games, Inc. All Rights Reserved.

#include "InflectionPoint.h"
#include "TDMGameModeBase.h"
#include "UI/HUD/InflectionPointHUD.h"
#include "Gameplay/Characters/BaseCharacter.h"
#include <string>
#include "Gameplay/Controllers/InflectionPointPlayerController.h"
#include "Gameplay/Characters/PlayerControlledFPSCharacter.h"

ATDMGameModeBase::ATDMGameModeBase()
	: Super() {
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnClassFinder(TEXT("/Game/InflectionPoint/Blueprints/Characters/PlayerCharacter"));
	DefaultPawnClass = PlayerPawnClassFinder.Class;

	// use our custom HUD class
	HUDClass = AInflectionPointHUD::StaticClass();
	PlayerControllerClass = AInflectionPointPlayerController::StaticClass();
}

void ATDMGameModeBase::UpdateMaxPlayers(FName SessioName) {
	IOnlineSessionPtr session = IOnlineSubsystem::Get()->GetSessionInterface();
	FOnlineSessionSettings* sessionSettings = session->GetSessionSettings(SessioName);
	if(sessionSettings) {
		MaxPlayers = sessionSettings->NumPublicConnections;
	} else {
		UE_LOG(LogTemp, Warning, TEXT("Warning: No session settings could be found, setting MaxPlayers to 2."));
		MaxPlayers = 2;
	}
}

void ATDMGameModeBase::StartMatch() {
	AssignTeamsAndPlayerStartGroups();
	StartNextRound();
}

void ATDMGameModeBase::StartNextRound() {
	OnRoundFinished(CurrentRound);
	CurrentRound++;
	for(FConstPlayerControllerIterator Iterator = GetWorld()->GetPlayerControllerIterator(); Iterator; ++Iterator) {
		auto playerController = UGameplayStatics::GetPlayerController(GetWorld(), Iterator.GetIndex());
		SpawnPlayer(playerController);
	}
}

void ATDMGameModeBase::PlayerDied(APlayerController * playerController) {
	if(CurrentRound == 0) {
		SpawnPlayer(playerController);
	} else if(IsRoundFinished()) {
		StartNextRound();
	} else {
		// TODO: Set Player as spectator
	}
}

bool ATDMGameModeBase::IsRoundFinished() {
	// TODO: Check if round ended
	return true;
}

void ATDMGameModeBase::SpawnPlayer(APlayerController * playerController) {
	AActor* playerStart = FindSpawnForPlayer(playerController);
	AssertNotNull(playerStart, GetWorld(), __FILE__, __LINE__);

	APawn* pawn = playerController->GetPawn();
	if(pawn) {
		pawn->SetLifeSpan(.0001);
	}

	FVector loc = playerStart->GetTransform().GetLocation();
	FRotator rot = FRotator(playerStart->GetTransform().GetRotation());

	FActorSpawnParameters ActorSpawnParams;
	ActorSpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	APlayerControlledFPSCharacter* newCharacter = GetWorld()->SpawnActor<APlayerControlledFPSCharacter>(DefaultPawnClass.Get(), loc, rot, ActorSpawnParams);

	playerController->Possess(newCharacter);
}

AActor* ATDMGameModeBase::FindSpawnForPlayer(APlayerController * playerController) {
	if(CurrentRound == 0)
		return FindPlayerStart(playerController);
	AInflectionPointPlayerController* ipPlayerController = Cast<AInflectionPointPlayerController>(playerController);
	return FindPlayerStart(ipPlayerController, GetSpawnTag(ipPlayerController));
}

FString ATDMGameModeBase::GetSpawnTag(AInflectionPointPlayerController*  playerController) {
	std::string spawnTagCString = std::to_string(playerController->Team) +
		playerController->PlayerStartGroup +
		std::to_string(CurrentRound);
	FString spawnTag(spawnTagCString.c_str());
	return spawnTag;
}

void ATDMGameModeBase::AssignTeamsAndPlayerStartGroups() {
	UWorld* world = GetWorld();

	for(auto iterator = world->GetPlayerControllerIterator(); iterator; ++iterator) {
		AInflectionPointPlayerController* controller = (AInflectionPointPlayerController*)UGameplayStatics::GetPlayerController(world, iterator.GetIndex());
		controller->Team = iterator.GetIndex() % 2 + 1;
		controller->PlayerStartGroup = 'A';
	}
}