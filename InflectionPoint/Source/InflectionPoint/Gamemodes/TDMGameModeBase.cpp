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
#include "Gameplay/InflectionPointGameInstanceBase.h"

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

	ScoreHandler = CreateDefaultSubobject<UTDMScoreHandler>(TEXT("ScoreHandler"));
	CharacterSpawner = CreateDefaultSubobject<UTDMCharacterSpawner>(TEXT("CharacterSpawner"));
}

void ATDMGameModeBase::PostLogin(APlayerController * NewPlayer) {
	Super::PostLogin(NewPlayer);
	if(NumPlayers >= MaxPlayers) {
		GameSession->KickPlayer(NewPlayer, FText::FromString("Server is already full!"));
		return;
	}
	NumPlayers++;
	UpdateCurrentPlayers(Cast<UInflectionPointGameInstanceBase>(GetGameInstance())->CurrentSessionName);
	APlayerControllerBase* controller = Cast<APlayerControllerBase>(NewPlayer);
	if(NumPlayers == MaxPlayers)
		StartMatch();
}

void ATDMGameModeBase::PreLogin(const FString & Options, const FString & Address, const FUniqueNetIdRepl & UniqueId, FString & ErrorMessage) {
	Super::PreLogin(Options, Address, UniqueId, ErrorMessage);
	if(NumPlayers >= MaxPlayers) {
		ErrorMessage = "Server is already full!";
	}
}

void ATDMGameModeBase::UpdateMaxPlayers(FName SessionName) {
	IOnlineSessionPtr session = IOnlineSubsystem::Get()->GetSessionInterface();
	FOnlineSessionSettings* sessionSettings = session->GetSessionSettings(SessionName);
	if(sessionSettings) {
		MaxPlayers = sessionSettings->NumPublicConnections;
	} else {
		UE_LOG(LogTemp, Warning, TEXT("Warning: No session settings could be found, setting MaxPlayers to %d."), OfflineMaxPlayers);
		MaxPlayers = OfflineMaxPlayers;
	}
}

void ATDMGameModeBase::UpdateCurrentPlayers(FName SessionName) {
	IOnlineSessionPtr session = IOnlineSubsystem::Get()->GetSessionInterface();
	FOnlineSessionSettings* sessionSettings = session->GetSessionSettings(SessionName);
	if(sessionSettings) {
		sessionSettings->Set(FName("CurrentPlayers"), NumPlayers, EOnlineDataAdvertisementType::ViaOnlineServiceAndPing);
		session->UpdateSession(SessionName, *sessionSettings);
	}
}

void ATDMGameModeBase::StartMatch() {
	GetGameState()->CurrentRound = 0;
	GetGameState()->MaxRoundNum = CharacterSpawner->GetSpawnPointCount() / MaxPlayers;
	CharacterSpawner->AssignTeamsAndPlayerStartGroups();
	ScoreHandler->ResetPlayerScores();
	StartTimer(this, GetWorld(), "StartNextRound", MatchStartDelay + 0.00001f, false); // we can't call "StartMatch" with a timer because that way the teams will not be replicated to the client before the characters are spawned 
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
	CharacterSpawner->SpawnPlayersAndReplays(GetGameState()->CurrentRound, PlayerRecordings);
	SendRoundStartedToPlayers(round);
	StartCountdown();
	StartTimer(this, GetWorld(), "StartSpawnCinematics", 0.3, false); // needed because rpc not redy ^^
}

void ATDMGameModeBase::StartSpawnCinematics() {
	ATDMLevelScriptBase* levelScript = Cast<ATDMLevelScriptBase>(GetWorld()->GetLevelScriptActor(GetLevel()));
	if(!levelScript) {
		return;
	}
	levelScript->MulticastStartSpawnCinematic();
}

void ATDMGameModeBase::CharacterDied(AController * KilledPlayer, AController* KillingPlayer, AActor* DamageCauser) {
	if(!AssertNotNull(KilledPlayer, GetWorld(), __FILE__, __LINE__))
		return;
	SendKillInfoToPlayers(KilledPlayer, KillingPlayer, DamageCauser);

	if(KilledPlayer->IsA(APlayerController::StaticClass()))
		Cast<ATDMPlayerStateBase>(KilledPlayer->PlayerState)->IsAlive = false;

	if(GetGameState()->CurrentRound > 0)
		ScoreHandler->AddKill(KilledPlayer, KillingPlayer);

	APlayerControllerBase* playerController = Cast<APlayerControllerBase>(KilledPlayer);
	if(!playerController)
		return;

	if(GetGameState()->CurrentRound > 0)
		SavePlayerRecordings(playerController);

	if(GetGameState()->CurrentRound == 0) {
		CharacterSpawner->SpawnAndPossessPlayer(playerController, 0);
	} else if(IsWinnerFound()) {
		StartTimer(this, GetWorld(), "EndCurrentRound", RoundEndDelay + 0.00001f, false); // 0 does not work o.O
	}
}

void ATDMGameModeBase::SendKillInfoToPlayers(AController * KilledPlayer, AController* KillingPlayer, AActor* DamageCauser) {
	FCharacterInfo killerInfo = KillingPlayer ? KillingPlayer->GetCharacter()->FindComponentByClass<UCharacterInfoProvider>()->GetCharacterInfo() : FCharacterInfo();
	FCharacterInfo killedInfo = KilledPlayer->GetCharacter()->FindComponentByClass<UCharacterInfoProvider>()->GetCharacterInfo();
	for(FConstPlayerControllerIterator Iterator = GetWorld()->GetPlayerControllerIterator(); Iterator; ++Iterator) {
		auto playerController = UGameplayStatics::GetPlayerController(GetWorld(), Iterator.GetIndex());
		APlayerControllerBase* controller = Cast<APlayerControllerBase>(playerController);
		float killedScoreChange = GetGameState()->CurrentRound == 0 ? 0 : ScoreHandler->GetKilledScoreChange(KilledPlayer, KillingPlayer);
		float killerScoreChange = GetGameState()->CurrentRound == 0 ? 0 : ScoreHandler->GetKillerScoreChange(KilledPlayer, KillingPlayer);
		auto weapon = Cast<ABaseWeapon>(DamageCauser);
		controller->ClientShowKillInfo(killedInfo, killedScoreChange, killerInfo, killerScoreChange, weapon ? weapon->WeaponTexture : NULL);
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
	return Cast<ATDMPlayerStateBase>(playerController->PlayerState)->IsAlive;
}

void ATDMGameModeBase::StartCountdown() {
	TArray<AActor*> foundActors;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), APlayerControllerBase::StaticClass(), foundActors);
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
		}
	}
	if(number == 0) {
		StartReplays();
	}
}

void ATDMGameModeBase::StartReplays() {
	TArray<AActor*> foundActors;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), AReplayCharacterBase::StaticClass(), foundActors);
	for(auto& item : foundActors)
		Cast<AReplayCharacterBase>(item)->StartReplay();
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
	DestroyAllActorsWithTag(FName("DeleteOnClearMap"));
}

void ATDMGameModeBase::DestroyAllActors(TSubclassOf<AActor> actorClass) {
	TArray<AActor*> foundActors;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), actorClass, foundActors);
	for(auto& item : foundActors)
		item->Destroy();
}

void ATDMGameModeBase::DestroyAllActorsWithTag(FName tag) {
	TArray<AActor*> foundActors;
	UGameplayStatics::GetAllActorsWithTag(GetWorld(), tag, foundActors);
	for(auto& item : foundActors)
		item->Destroy();
}