// Copyright 1998-2016 Epic Games, Inc. All Rights Reserved.

#include "InflectionPoint.h"
#include "TDMGameModeBase.h"
#include <string>
#include "Gameplay/Characters/BaseCharacter.h"
#include "Gameplay/Damage/MortalityProvider.h"
#include "Gameplay/Weapons/InflectionPointProjectile.h"
#include "Gameplay/Controllers/PlayerControllerBase.h"
#include "Gameplay/Controllers/AIControllerBase.h"
#include "Gameplay/Characters/PlayerCharacterBase.h"
#include "Gameplay/Characters/ReplayCharacterBase.h"
#include "Gamemodes/TDMPlayerStateBase.h"
#include "Gamemodes/TDMLevelScriptBase.h"

ATDMGameModeBase::ATDMGameModeBase()
	: Super() {
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnClassFinder(TEXT("/Game/InflectionPoint/Blueprints/Characters/PlayerCharacter"));
	static ConstructorHelpers::FClassFinder<APlayerController> PlayerControllerClassFinder(TEXT("/Game/InflectionPoint/Blueprints/Controllers/PlayerController"));
	static ConstructorHelpers::FClassFinder<ATDMGameStateBase> GameStateClassFinder(TEXT("/Game/InflectionPoint/Blueprints/GameModes/TDMGameState"));

	DefaultPawnClass = PlayerPawnClassFinder.Class;
	GameStateClass = GameStateClassFinder.Class;

	// configure default classes
	PlayerControllerClass = PlayerControllerClassFinder.Class;
	PlayerStateClass = ATDMPlayerStateBase::StaticClass();
}

void ATDMGameModeBase::PostLogin(APlayerController * NewPlayer) {
	Super::PostLogin(NewPlayer);
	NumPlayers++;
	APlayerControllerBase* controller = Cast<APlayerControllerBase>(NewPlayer);
	controller->ClientRoundStarted(0);
	if(NumPlayers == MaxPlayers)
		StartMatch();
}


void ATDMGameModeBase::UpdateMaxPlayers(FName SessioName) {
	IOnlineSessionPtr session = IOnlineSubsystem::Get()->GetSessionInterface();
	FOnlineSessionSettings* sessionSettings = session->GetSessionSettings(SessioName);
	if(sessionSettings) {
		MaxPlayers = sessionSettings->NumPublicConnections;
	} else {
		UE_LOG(LogTemp, Warning, TEXT("Warning: No session settings could be found, setting MaxPlayers to %d."), OfflineMaxPlayers);
		MaxPlayers = OfflineMaxPlayers;
	}
}

void ATDMGameModeBase::StartMatch() {
	GetGameState()->CurrentRound = 0;
	AssignTeamsAndPlayerStartGroups();
	ResetPlayerScores();
	StartNextRound();
}

void ATDMGameModeBase::EndCurrentRound() {
	SaveRecordingsFromRemainingPlayers();
	StartNextRound();
}

void ATDMGameModeBase::StartNextRound() {
	int round = GetGameState()->CurrentRound + 1;
	if(round > GetGameState()->MaxRoundNum)
		round = 1; // restart 
	GetGameState()->CurrentRound = round;
	ClearMap();
	SpawnPlayersAndReplays();
	SendRoundStartedToPlayers(round);
	StartCountdown();
	StartTimer(this, GetWorld(), "StartSpawnCinematics", 0.3, false); // needed because rpc not redy ^^
}

void ATDMGameModeBase::StartSpawnCinematics() {
	ATDMLevelScriptBase* levelScript = Cast<ATDMLevelScriptBase>(GetWorld()->GetLevelScriptActor(GetLevel()));
	if(!levelScript) {
		SoftAssertTrue(false, GetWorld(), __FILE__, __LINE__, "Could not play SpawnCinematic");
		return;
	}
	levelScript->MulticastStartSpawnCinematic();
}

void ATDMGameModeBase::CharacterDied(AController * KilledPlayer, AController* KillingPlayer, AActor* DamageCauser) {
	SendKillInfoToPlayers(KilledPlayer, KillingPlayer, DamageCauser);

	if(GetGameState()->CurrentRound > 0)
		WriteKillToPlayerStates(KilledPlayer, KillingPlayer);

	APlayerControllerBase* playerController = Cast<APlayerControllerBase>(KilledPlayer);
	if(!playerController)
		return;

	if(GetGameState()->CurrentRound > 0)
		SavePlayerRecordings(playerController);


	if(GetGameState()->CurrentRound == 0) {
		SpawnAndPossessPlayer(playerController);
	} else if(IsWinnerFound()) {
		StartTimer(this, GetWorld(), "EndCurrentRound", RoundEndDelay + 0.00001f, false); // 0 does not work o.O
	}
}


void ATDMGameModeBase::ResetPlayerScores() {
	for(FConstPlayerControllerIterator Iterator = GetWorld()->GetPlayerControllerIterator(); Iterator; ++Iterator) {
		auto playerController = UGameplayStatics::GetPlayerController(GetWorld(), Iterator.GetIndex());
		Cast<ATDMPlayerStateBase>(playerController->PlayerState)->ResetScore();
	}
}

void ATDMGameModeBase::WriteKillToPlayerStates(AController * KilledPlayer, AController* KillingPlayer) {
	UCharacterInfoProvider* killerInfo = KillingPlayer ? KillingPlayer->FindComponentByClass<UCharacterInfoProvider>() : NULL;
	UCharacterInfoProvider* killedInfo = KilledPlayer->FindComponentByClass<UCharacterInfoProvider>();

	if(!killedInfo->IsReplay)
		Cast<ATDMPlayerStateBase>(KilledPlayer->PlayerState)->AddDeath();

	if(killerInfo && !killerInfo->IsReplay) {
		if(Cast<ATDMPlayerStateBase>(killedInfo->PlayerState)->Team == Cast<ATDMPlayerStateBase>(killerInfo->PlayerState)->Team) {
			Cast<ATDMPlayerStateBase>(KillingPlayer->PlayerState)->AddTeamKill();
		} else if(!killedInfo->IsReplay) {
			Cast<ATDMPlayerStateBase>(KillingPlayer->PlayerState)->AddPlayerKill();
		} else {
			Cast<ATDMPlayerStateBase>(KillingPlayer->PlayerState)->AddReplayKill();
		}
	}
}

void ATDMGameModeBase::SendKillInfoToPlayers(AController * KilledPlayer, AController* KillingPlayer, AActor* DamageCauser) {
	FCharacterInfo killerInfo = KillingPlayer ? KillingPlayer->FindComponentByClass<UCharacterInfoProvider>()->GetCharacterInfo() : FCharacterInfo();
	FCharacterInfo killedInfo = KilledPlayer->FindComponentByClass<UCharacterInfoProvider>()->GetCharacterInfo();
	for(FConstPlayerControllerIterator Iterator = GetWorld()->GetPlayerControllerIterator(); Iterator; ++Iterator) {
		auto playerController = UGameplayStatics::GetPlayerController(GetWorld(), Iterator.GetIndex());
		APlayerControllerBase* controller = Cast<APlayerControllerBase>(playerController);
		controller->ClientShowKillInfo(killerInfo, killedInfo, NULL);
	}
}

void ATDMGameModeBase::SendRoundStartedToPlayers(int Round) {
	for(FConstPlayerControllerIterator Iterator = GetWorld()->GetPlayerControllerIterator(); Iterator; ++Iterator) {
		auto playerController = UGameplayStatics::GetPlayerController(GetWorld(), Iterator.GetIndex());
		APlayerControllerBase* controller = Cast<APlayerControllerBase>(playerController);
		controller->ClientRoundStarted(Round);
	}
}

bool ATDMGameModeBase::IsWinnerFound() {
	return (GetTeamsAlive().Num() == 1);
}

TArray<int> ATDMGameModeBase::GetTeamsAlive() {
	TArray<int> teamsAlive;
	for(FConstPlayerControllerIterator Iterator = GetWorld()->GetPlayerControllerIterator(); Iterator; ++Iterator) {
		auto playerController = UGameplayStatics::GetPlayerController(GetWorld(), Iterator.GetIndex());
		auto ipPlayerController = Cast<APlayerControllerBase>(playerController);
		auto playerState = Cast<ATDMPlayerStateBase>(playerController->PlayerState);

		if(teamsAlive.Contains(playerState->Team))
			continue;
		if(IsPlayerAlive(ipPlayerController))
			teamsAlive.Add(playerState->Team);
	}
	return teamsAlive;
}


bool ATDMGameModeBase::IsPlayerAlive(APlayerControllerBase* playerController) {
	auto pawn = playerController->GetPawn();
	if(!pawn)
		return false;

	auto mortalityProvider = pawn->FindComponentByClass<UMortalityProvider>();
	if(mortalityProvider && mortalityProvider->CurrentHealth > 0)
		return true;

	return false;
}

void ATDMGameModeBase::SpawnPlayersAndReplays() {
	for(FConstPlayerControllerIterator Iterator = GetWorld()->GetPlayerControllerIterator(); Iterator; ++Iterator) {
		auto playerController = UGameplayStatics::GetPlayerController(GetWorld(), Iterator.GetIndex());
		auto ipPlayerController = Cast<APlayerControllerBase>(playerController);
		SpawnAndPossessPlayer(ipPlayerController);
		for(int i = 1; i < GetGameState()->CurrentRound; i++)
			SpawnAndPrepareReplay(ipPlayerController, i);
	}
}

void ATDMGameModeBase::StartCountdown() {
	TArray<AActor*> foundActors;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), APlayerControllerBase::StaticClass(), foundActors);
	for(auto& controller : foundActors) {
		APlayerControllerBase* playerController = Cast<APlayerControllerBase>(controller);
		//playerController->ClientSetIgnoreInput(true);
	}
	for(int i = CountDownDuration; i >= 0; i--) {
		StartTimer(this, GetWorld(), "UpdateCountdown", (CountDownDuration - i + 1), false, foundActors, i);
	}
}

void ATDMGameModeBase::UpdateCountdown(TArray<AActor*> controllers, int number) {
	for(auto& controller : controllers) {
		APlayerControllerBase* playerController = Cast<APlayerControllerBase>(controller);
		playerController->ClientShowCountdownNumber(number);
		if(number == 0) {
			playerController->GetCharacter()->FindComponentByClass<UPlayerStateRecorder>()->ServerStartRecording();
			playerController->ClientSetIgnoreInput(false);
		}
	}
	if(number == 0) {
		StartReplays();
	}
}

template <typename CharacterType>
CharacterType* ATDMGameModeBase::SpawnCharacter(UClass* spawnClass, APlayerControllerBase * playerController, AActor* playerStart) {
	FVector loc = playerStart->GetTransform().GetLocation();
	FRotator rot = FRotator(playerStart->GetTransform().GetRotation());

	FActorSpawnParameters ActorSpawnParams;
	ActorSpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	CharacterType* newCharacter = GetWorld()->SpawnActor<CharacterType>(spawnClass, loc, rot, ActorSpawnParams);
	AssertNotNull(newCharacter, GetWorld(), __FILE__, __LINE__, "Could not spawn character!");
	return newCharacter;
}

void ATDMGameModeBase::SpawnAndPossessPlayer(APlayerControllerBase * playerController) {
	AActor* spawnPoint = FindSpawnForPlayer(playerController, GetGameState()->CurrentRound);
	AssertNotNull(spawnPoint, GetWorld(), __FILE__, __LINE__);

	auto character = SpawnCharacter<APlayerCharacterBase>(DefaultPawnClass.Get(), playerController, spawnPoint);

	playerController->ClientSetControlRotation(FRotator(spawnPoint->GetTransform().GetRotation()));
	playerController->Possess(character);

	if(GetGameState()->CurrentRound == 0) {
		playerController->ClientRoundStarted(0);
	}
}

void ATDMGameModeBase::SpawnAndPrepareReplay(APlayerControllerBase* playerController, int round) {
	AssertTrue(PlayerRecordings.Contains(playerController), GetWorld(), __FILE__, __LINE__, "Could not find replay");
	auto spawnPoint = FindSpawnForPlayer(playerController, round);
	AssertNotNull(spawnPoint, GetWorld(), __FILE__, __LINE__, "No spawn found");
	auto character = SpawnCharacter<AReplayCharacterBase>(ReplayCharacter, playerController, spawnPoint);

	AssertTrue(PlayerRecordings[playerController].Contains(round), GetWorld(), __FILE__, __LINE__, "Could not find replay");

	// Start Replay on spawned ReplayCharacter
	if(PlayerRecordings[playerController].Contains(round))
		character->SetReplayData(PlayerRecordings[playerController][round]);

	Cast<AAIControllerBase>(character->GetController())->Initialize(playerController);
}

void ATDMGameModeBase::StartReplays() {
	TArray<AActor*> foundActors;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), AReplayCharacterBase::StaticClass(), foundActors);
	for(auto& item : foundActors)
		Cast<AReplayCharacterBase>(item)->StartReplay();
}

AActor* ATDMGameModeBase::FindSpawnForPlayer(APlayerControllerBase * playerController, int round) {
	if(round == 0)
		return FindPlayerStart(playerController);
	return FindPlayerStart(playerController, GetSpawnTag(playerController, round));
}

FString ATDMGameModeBase::GetSpawnTag(APlayerControllerBase*  playerController, int round) {
	auto playerState = Cast<ATDMPlayerStateBase>(playerController->PlayerState);
	FString spawnTag = FString::FromInt(playerState->Team) + playerState->PlayerStartGroup + FString::FromInt(round);
	return spawnTag;
}

void ATDMGameModeBase::AssignTeamsAndPlayerStartGroups() {
	UWorld* world = GetWorld();

	for(auto iterator = world->GetPlayerControllerIterator(); iterator; ++iterator) {
		APlayerControllerBase* controller = (APlayerControllerBase*)UGameplayStatics::GetPlayerController(world, iterator.GetIndex());
		ATDMPlayerStateBase* playerState = Cast<ATDMPlayerStateBase>(controller->PlayerState);
		playerState->Team = iterator.GetIndex() % 2 + 1;
		playerState->PlayerStartGroup = TEXT("A");
	}
}

void ATDMGameModeBase::SaveRecordingsFromRemainingPlayers() {
	for(FConstPlayerControllerIterator Iterator = GetWorld()->GetPlayerControllerIterator(); Iterator; ++Iterator) {
		auto playerController = UGameplayStatics::GetPlayerController(GetWorld(), Iterator.GetIndex());
		auto ipPlayerController = Cast<APlayerControllerBase>(playerController);
		if(IsPlayerAlive(ipPlayerController))
			SavePlayerRecordings(ipPlayerController);
	}
}

void ATDMGameModeBase::SavePlayerRecordings(APlayerControllerBase * playerController) {
	auto pawn = playerController->GetPawn();
	if(AssertNotNull(pawn, GetWorld(), __FILE__, __LINE__)) {
		auto playerStateRecorder = pawn->FindComponentByClass<UPlayerStateRecorder>();
		AssertNotNull(playerStateRecorder, GetWorld(), __FILE__, __LINE__);
		if(!PlayerRecordings.Contains(playerController)) {
			TMap<int, TArray<FRecordedPlayerState>> map;
			PlayerRecordings.Add(playerController, map);
		}
		PlayerRecordings[playerController].Add(GetGameState()->CurrentRound, playerStateRecorder->RecordedPlayerStates);
	}
}

void ATDMGameModeBase::ClearMap() {
	DestroyAllActors(AReplayCharacterBase::StaticClass());
	DestroyAllActors(APlayerCharacterBase::StaticClass());
	DestroyAllActors(AInflectionPointProjectile::StaticClass());
}

void ATDMGameModeBase::DestroyAllActors(TSubclassOf<AActor> actorClass) {
	TArray<AActor*> foundActors;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), actorClass, foundActors);
	for(auto& item : foundActors)
		item->Destroy();
}