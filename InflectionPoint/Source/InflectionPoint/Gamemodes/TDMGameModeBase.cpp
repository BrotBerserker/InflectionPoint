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
	PrimaryActorTick.bCanEverTick = true;

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

void ATDMGameModeBase::Tick(float DeltaSeconds) {
	Super::Tick(DeltaSeconds);

	UpdateTimeUntilMatchStart(DeltaSeconds);

	UpdateTimeUntilNextCountdownUpdate(DeltaSeconds);
}

void ATDMGameModeBase::UpdateTimeUntilNextCountdownUpdate(float DeltaSeconds) {
	if(GetGameState()->CurrentRound == 0 || nextCountdownNumber < 0) {
		return;
	}
	timeUntilNextCountdownUpdate -= DeltaSeconds;
	if(timeUntilNextCountdownUpdate <= 0) {
		timeUntilNextCountdownUpdate = 1.f;
		UpdateCountdown(nextCountdownNumber--);
	}
}

void ATDMGameModeBase::UpdateTimeUntilMatchStart(float DeltaSeconds) {
	if(GetGameState()->CurrentRound > 0) {
		return;
	}
	if(GetGameState()->NumPlayers == GetGameState()->MaxPlayers) {
		timeUntilMatchStart -= DeltaSeconds;
	} else {
		timeUntilMatchStart = MatchStartDelay;
	}
	if(timeUntilMatchStart <= 0) {
		timeUntilMatchStart = MatchStartDelay;
		StartMatch();
	}
}

void ATDMGameModeBase::PostLogin(APlayerController * NewPlayer) {
	Super::PostLogin(NewPlayer);
	AssertNotNull(GetGameState(), GetWorld(), __FILE__, __LINE__);
	if(GetGameState()->NumPlayers >= GetGameState()->MaxPlayers) {
		GameSession->KickPlayer(NewPlayer, FText::FromString("Server is already full!"));
		return;
	}
	GetGameState()->NumPlayers++;
	UpdateCurrentPlayers(Cast<UInflectionPointGameInstanceBase>(GetGameInstance())->CurrentSessionName);
}

void ATDMGameModeBase::PreLogin(const FString & Options, const FString & Address, const FUniqueNetIdRepl & UniqueId, FString & ErrorMessage) {
	Super::PreLogin(Options, Address, UniqueId, ErrorMessage);
	AssertNotNull(GetGameState(), GetWorld(), __FILE__, __LINE__);
	if(GetGameState()->NumPlayers >= GetGameState()->MaxPlayers) {
		ErrorMessage = "Server is already full!";
	}
}

void ATDMGameModeBase::Logout(AController* Exiting) {
	Super::Logout(Exiting);
	GetGameState()->NumPlayers--;
	UpdateCurrentPlayers(Cast<UInflectionPointGameInstanceBase>(GetGameInstance())->CurrentSessionName);
	if(GetGameState()->CurrentRound > 0 && nextCountdownNumber < 0 && IsWinnerFound(Exiting) && !isPlayingEndMatchSequence) {
		StartEndMatchSequence();
	}
}

void ATDMGameModeBase::InitializeSettings(FName SessionName) {
	IOnlineSessionPtr session = IOnlineSubsystem::Get()->GetSessionInterface();
	FOnlineSessionSettings* sessionSettings = session->GetSessionSettings(SessionName);
	AssertNotNull(GetGameState(), GetWorld(), __FILE__, __LINE__);
	if(sessionSettings) {
		GetGameState()->MaxPlayers = sessionSettings->NumPublicConnections;
		sessionSettings->Get(FName("Rounds"), GetGameState()->MaxRoundNum);
	} else {
		UE_LOG(LogTemp, Warning, TEXT("Warning: No session settings could be found, using offline settings."));
		GetGameState()->MaxPlayers = OfflineMaxPlayers;
		GetGameState()->MaxRoundNum = OfflineMaxRoundNum;
	}
}

void ATDMGameModeBase::UpdateCurrentPlayers(FName SessionName) {
	IOnlineSessionPtr session = IOnlineSubsystem::Get()->GetSessionInterface();
	FOnlineSessionSettings* sessionSettings = session->GetSessionSettings(SessionName);
	if(sessionSettings) {
		sessionSettings->Set(FName("CurrentPlayers"), GetGameState()->NumPlayers, EOnlineDataAdvertisementType::ViaOnlineServiceAndPing);
		session->UpdateSession(SessionName, *sessionSettings);
	}
}

void ATDMGameModeBase::StartMatch() {
	ResetGameState();
	CharacterSpawner->AssignTeamsAndPlayerStartGroups();
	StartNextRound();
}

void ATDMGameModeBase::ReStartMatch() {
	isPlayingEndMatchSequence = false;
	ResetGameState();
	ClearMap();
	CharacterSpawner->SpawnAllPlayersForWarmupRound();
	CharacterSpawner->AssignTeamsAndPlayerStartGroups();
}

void ATDMGameModeBase::ResetGameState() {
	GetGameState()->TeamWins.Init(0, GetGameState()->TeamCount + 1); // +1 because teams start with 1
	GetGameState()->CurrentRound = 0;
	GetGameState()->CurrentPhase = 0;
	GetGameState()->MaxPhaseNum = CharacterSpawner->GetSpawnPointCount() / GetGameState()->MaxPlayers;
	GetGameState()->ResetPlayerScores();
	GetGameState()->ResetTotalPlayerScores();
}

void ATDMGameModeBase::EndCurrentPhase() {
	SaveRecordingsFromRemainingPlayers();
	if(GetGameState()->CurrentPhase == GetGameState()->MaxPhaseNum) {
		EndCurrentRound();
	} else {
		StartNextPhase();
	}
}

void ATDMGameModeBase::StartNextPhase() {
	int phase = GetGameState()->CurrentPhase + 1;
	if(!AssertTrue(phase <= GetGameState()->MaxPhaseNum, GetWorld(), __FILE__, __LINE__, "Cant start the next Phase"))
		return;
	GetGameState()->CurrentPhase = phase;
	ClearMap();
	CharacterSpawner->SpawnPlayersAndReplays(GetGameState()->CurrentPhase, PlayerRecordings);
	SendPhaseStartedToPlayers(phase);
	StartCountdown();
	StartTimer(this, GetWorld(), "StartSpawnCinematics", 0.3, false); // needed because rpc not redy ^^
}

void ATDMGameModeBase::EndCurrentRound() {
	ScoreHandler->SelectWinnerTeamForRound();
	ScoreHandler->UpdateScoresForNextRound();
	if(GetGameState()->CurrentRound >= GetGameState()->MaxRoundNum) {
		StartEndMatchSequence();
		return;
	}
	StartNextRound();
}

void ATDMGameModeBase::StartEndMatchSequence() {
	GetGameState()->ResetPlayerScores(); // so all players appear alive
	ScoreHandler->SetCurrentScoresToTotalScore();
	isPlayingEndMatchSequence = true;
	int winningTeam = ScoreHandler->GetWinningTeam();
	int losingTeam = ScoreHandler->GetLosingTeam();
	NotifyControllersOfEndMatch(winningTeam);
	StartTimer(this, GetWorld(), "ReStartMatch", MatchReStartDelay, false);

	// if no levelscript is provided, just restart the match without playing an end match sequence
	ATDMLevelScriptBase* levelScript = Cast<ATDMLevelScriptBase>(GetWorld()->GetLevelScriptActor(GetLevel()));
	if(!levelScript) {
		return;
	}
	ClearMap();
	FString winnerName = GetAnyPlayerControllerInTeam(winningTeam) ? GetAnyPlayerControllerInTeam(winningTeam)->PlayerState->GetPlayerName() : "oops something went wrong";
	FString loserName = GetAnyPlayerControllerInTeam(losingTeam) ? GetAnyPlayerControllerInTeam(losingTeam)->PlayerState->GetPlayerName() : "oops something went wrong";
	levelScript->StartEndMatchSequence(CharacterSpawner->PlayerCharacters[winningTeam], CharacterSpawner->PlayerCharacters[losingTeam], winnerName, loserName);
}

void ATDMGameModeBase::NotifyControllersOfEndMatch(int winnerTeam) {
	TArray<AActor*> controllers;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), APlayerControllerBase::StaticClass(), controllers);
	for(auto& controller : controllers) {
		APlayerControllerBase* playerController = Cast<APlayerControllerBase>(controller);
		playerController->ClientShowMatchEnd(winnerTeam);
	}
}

void ATDMGameModeBase::StartNextRound() {
	if(!AssertTrue(GetGameState()->CurrentRound < GetGameState()->MaxRoundNum, GetWorld(), __FILE__, __LINE__, "Cant Start next Round"))
		return;
	GetGameState()->ResetPlayerScores();
	GetGameState()->CurrentPhase = 0;
	GetGameState()->CurrentRound++;
	StartNextPhase();
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

	if(GetGameState()->CurrentPhase > 0)
		ScoreHandler->AddKill(KilledPlayer, KillingPlayer);

	APlayerControllerBase* playerController = Cast<APlayerControllerBase>(KilledPlayer);
	if(!playerController)
		return;

	if(GetGameState()->CurrentPhase > 0)
		SavePlayerRecordings(playerController);

	if(GetGameState()->CurrentPhase == 0) {
		CharacterSpawner->SpawnAndPossessPlayer(playerController, 0);
	} else if(IsWinnerFound()) {
		StartTimer(this, GetWorld(), "EndCurrentPhase", PhaseEndDelay + 0.00001f, false); // 0 does not work o.O
	}
}

void ATDMGameModeBase::SendKillInfoToPlayers(AController * KilledPlayer, AController* KillingPlayer, AActor* DamageCauser) {
	FCharacterInfo killerInfo = KillingPlayer ? KillingPlayer->GetCharacter()->FindComponentByClass<UCharacterInfoProvider>()->GetCharacterInfo() : FCharacterInfo();
	FCharacterInfo killedInfo = KilledPlayer->GetCharacter()->FindComponentByClass<UCharacterInfoProvider>()->GetCharacterInfo();
	for(FConstPlayerControllerIterator Iterator = GetWorld()->GetPlayerControllerIterator(); Iterator; ++Iterator) {
		auto playerController = UGameplayStatics::GetPlayerController(GetWorld(), Iterator.GetIndex());
		APlayerControllerBase* controller = Cast<APlayerControllerBase>(playerController);
		float killedScoreChange = GetGameState()->CurrentPhase == 0 ? 0 : ScoreHandler->GetKilledScoreChange(KilledPlayer, KillingPlayer);
		float killerScoreChange = GetGameState()->CurrentPhase == 0 ? 0 : ScoreHandler->GetKillerScoreChange(KilledPlayer, KillingPlayer);
		auto weapon = Cast<ABaseWeapon>(DamageCauser);
		controller->ClientShowKillInfo(killedInfo, killedScoreChange, killerInfo, killerScoreChange, weapon ? weapon->WeaponTexture : NULL);
	}
}

void ATDMGameModeBase::SendPhaseStartedToPlayers(int Phase) {
	for(FConstPlayerControllerIterator Iterator = GetWorld()->GetPlayerControllerIterator(); Iterator; ++Iterator) {
		auto playerController = UGameplayStatics::GetPlayerController(GetWorld(), Iterator.GetIndex());
		APlayerControllerBase* controller = Cast<APlayerControllerBase>(playerController);
		controller->ClientPhaseStarted(Phase);
	}
}

bool ATDMGameModeBase::IsWinnerFound(AController* controllerToIgnore) {
	return (GetTeamsAlive(controllerToIgnore).Num() == 1);
}

TArray<int> ATDMGameModeBase::GetTeamsAlive(AController* controllerToIgnore) {
	TArray<int> teamsAlive;
	for(FConstPlayerControllerIterator Iterator = GetWorld()->GetPlayerControllerIterator(); Iterator; ++Iterator) {
		auto playerController = UGameplayStatics::GetPlayerController(GetWorld(), Iterator.GetIndex());
		auto ipPlayerController = Cast<APlayerControllerBase>(playerController);
		auto playerState = Cast<ATDMPlayerStateBase>(playerController->PlayerState);

		if(ipPlayerController == controllerToIgnore || teamsAlive.Contains(playerState->Team))
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
	nextCountdownNumber = CountDownDuration;
	timeUntilNextCountdownUpdate = 1.f;
}

void ATDMGameModeBase::UpdateCountdown(int number) {
	TArray<AActor*> controllers;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), APlayerControllerBase::StaticClass(), controllers);
	for(auto& controller : controllers) {
		APlayerControllerBase* playerController = Cast<APlayerControllerBase>(controller);
		playerController->ClientShowCountdownNumber(number);
		if(number == 0 && playerController->GetCharacter()) {
			playerController->GetCharacter()->FindComponentByClass<UPlayerStateRecorder>()->ServerStartRecording();
		}
	}
	if(number == 0) {
		StartReplays();
		if(IsWinnerFound()) {
			StartTimer(this, GetWorld(), "StartEndMatchSequence", 1.1f, false); // wait for countdown animation
		}
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
		PlayerRecordings[playerController].Add(GetGameState()->CurrentPhase, playerStateRecorder->RecordedPlayerStates);
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

APlayerController* ATDMGameModeBase::GetAnyPlayerControllerInTeam(int team) {
	for(FConstPlayerControllerIterator Iterator = GetWorld()->GetPlayerControllerIterator(); Iterator; ++Iterator) {
		auto playerController = UGameplayStatics::GetPlayerController(GetWorld(), Iterator.GetIndex());
		ATDMPlayerStateBase* playerState = Cast<ATDMPlayerStateBase>(playerController->PlayerState);
		if(playerState && playerState->Team == team) {
			return playerController;
		}
	}
	return nullptr;
}