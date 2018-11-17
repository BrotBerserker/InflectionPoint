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

	MatchStartCountdown = CreateDefaultSubobject<UCountdown>(TEXT("MatchStartCountdown"));
	PhaseStartCountdown = CreateDefaultSubobject<UCountdown>(TEXT("PhaseStartCountdown"));
	ShopCountdown = CreateDefaultSubobject<UCountdown>(TEXT("ShopCountdown"));
}

void ATDMGameModeBase::PostInitializeComponents() {
	Super::PostInitializeComponents();
	MatchStartCountdown->Setup(this, &ATDMGameModeBase::UpdateMatchCountdown, &ATDMGameModeBase::StartMatch, MatchStartDelay);
	ShopCountdown->Setup(this, &ATDMGameModeBase::UpdateShopCountdown, &ATDMGameModeBase::PreparePhaseStart, ShopTime, true);
	PhaseStartCountdown->Setup(this, &ATDMGameModeBase::UpdatePhaseCountdown, &ATDMGameModeBase::StartPhase, PhaseStartDelay);
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

void ATDMGameModeBase::PreLogin(const FString & Options, const FString & Address, const FUniqueNetIdRepl & UniqueId, FString & ErrorMessage) {
	Super::PreLogin(Options, Address, UniqueId, ErrorMessage);
	AssertNotNull(GetGameState(), GetWorld(), __FILE__, __LINE__);
	if(GetGameState()->NumPlayers >= GetGameState()->MaxPlayers) {
		ErrorMessage = "Server is already full!";
	}
}

void ATDMGameModeBase::PostLogin(APlayerController * NewPlayer) {
	Super::PostLogin(NewPlayer);
	AssertNotNull(GetGameState(), GetWorld(), __FILE__, __LINE__);
	GetGameState()->NumPlayers++;
	UpdateCurrentPlayers(Cast<UInflectionPointGameInstanceBase>(GetGameInstance())->CurrentSessionName);
	if(GetGameState()->NumPlayers > GetGameState()->MaxPlayers) {
		GameSession->KickPlayer(NewPlayer, FText::FromString("Server is already full!"));
		return;
	}
	if(GetGameState()->NumPlayers == GetGameState()->MaxPlayers) {
		MatchStartCountdown->Start();
	}
}

void ATDMGameModeBase::Logout(AController* Exiting) {
	Super::Logout(Exiting);
	GetGameState()->NumPlayers--;
	UpdateCurrentPlayers(Cast<UInflectionPointGameInstanceBase>(GetGameInstance())->CurrentSessionName);
	MatchStartCountdown->Stop();
	PhaseStartCountdown->Stop();
	if(GetGameState()->CurrentRound > 0 && IsPhaseWinnerFound(Exiting) && !isPlayingEndMatchSequence) {
		StartEndMatchSequence();
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
	GetGameState()->PrepareForMatchStart(CharacterSpawner->GetSpawnPointCount());
	CharacterSpawner->AssignTeamsAndPlayerStartGroups();
	StartNextRound();
}

void ATDMGameModeBase::StartNextRound() {
	if(!AssertTrue(GetGameState()->CurrentRound < GetGameState()->MaxRoundNum, GetWorld(), __FILE__, __LINE__, "Cant Start next Round"))
		return;
	GetGameState()->PrepareForRoundStart();
	PlayerRecordings.Reset();
	StartNextPhase();
}

void ATDMGameModeBase::StartNextPhase() {
	int phase = GetGameState()->CurrentPhase + 1;
	if(!AssertTrue(phase <= GetGameState()->MaxPhaseNum, GetWorld(), __FILE__, __LINE__, "Cant start the next Phase"))
		return;
	GetGameState()->CurrentPhase = phase;
	ResetLevel();
	ShowShops();
	ShopCountdown->Start();
}

void ATDMGameModeBase::ShowShops() {
	bool isFirstPhaseInRound = GetGameState()->CurrentPhase == 1;
	DoShitForAllPlayerControllers(GetWorld(), [&](APlayerControllerBase* controller) {
		controller->ClientShowShop(isFirstPhaseInRound);
	});
}

void ATDMGameModeBase::PreparePhaseStart() {
	CharacterSpawner->SpawnPlayersAndReplays(GetGameState()->CurrentPhase, PlayerRecordings);
	SendPhaseStartedToPlayers(GetGameState()->CurrentPhase);
	PhaseStartCountdown->Start();
	StartTimer(this, GetWorld(), "StartSpawnCinematics", 0.3, false); // needed because rpc not redy ^^
}

void ATDMGameModeBase::StartPhase() {
	StartReplays();
	if(IsPhaseWinnerFound()) {
		StartTimer(this, GetWorld(), "StartEndMatchSequence", 1.1f, false); // wait for countdown animation
	}
}

void ATDMGameModeBase::EndCurrentPhase() {
	SaveRecordingsFromRemainingPlayers();
	if(GetGameState()->CurrentPhase == GetGameState()->MaxPhaseNum) {
		EndCurrentRound();
	} else {
		StartNextPhase();
	}
}

void ATDMGameModeBase::EndCurrentRound() {
	int winnerTeam = ScoreHandler->SelectWinnerTeamForRound();
	ScoreHandler->UpdateScoresForNextRound();
	if(IsMatchWinnerFound()) {
		StartEndMatchSequence();
		return;
	}
	NotifyControllersOfEndRound(winnerTeam);
	StartTimer(this, GetWorld(), "StartNextRound", RoundEndDelay, false);
}

void ATDMGameModeBase::ReStartMatch() {
	isPlayingEndMatchSequence = false;
	GetGameState()->PrepareForMatchStart(CharacterSpawner->GetSpawnPointCount());
	ResetLevel();
	CharacterSpawner->SpawnAllPlayersForWarmupRound();
	if(GetGameState()->NumPlayers == GetGameState()->MaxPlayers) {
		MatchStartCountdown->Start();
	}
}

void ATDMGameModeBase::UpdateMatchCountdown(int number) {
	DoShitForAllPlayerControllers(GetWorld(), [&](APlayerControllerBase* controller) {
		controller->ClientShowMatchCountdownNumber(number);
	});
}

void ATDMGameModeBase::UpdatePhaseCountdown(int number) {
	DoShitForAllPlayerControllers(GetWorld(), [&](APlayerControllerBase* controller) {
		controller->ClientShowPhaseCountdownNumber(number);
	});
}

void ATDMGameModeBase::UpdateShopCountdown(int number) {
	DoShitForAllPlayerControllers(GetWorld(), [&](APlayerControllerBase* controller) {
		controller->ClientShowShopCountdownNumber(number);
	});
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
	ResetLevel();
	FString winnerName = GetAnyPlayerControllerInTeam(winningTeam) ? GetAnyPlayerControllerInTeam(winningTeam)->PlayerState->GetPlayerName() : "oops something went wrong";
	FString loserName = GetAnyPlayerControllerInTeam(losingTeam) ? GetAnyPlayerControllerInTeam(losingTeam)->PlayerState->GetPlayerName() : "oops something went wrong";
	levelScript->StartEndMatchSequence(CharacterSpawner->PlayerCharacters[winningTeam], CharacterSpawner->PlayerCharacters[losingTeam], winnerName, loserName);
}

void ATDMGameModeBase::NotifyControllersOfEndMatch(int winnerTeam) {
	DoShitForAllPlayerControllers(GetWorld(), [&](APlayerControllerBase* controller) {
		controller->ClientShowMatchEnd(winnerTeam);
	});
}

void ATDMGameModeBase::NotifyControllersOfEndRound(int winnerTeam) {
	DoShitForAllPlayerControllers(GetWorld(), [&](APlayerControllerBase* controller) {
		controller->ClientShowRoundEnd(winnerTeam);
	});
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
	} else if(IsPhaseWinnerFound()) {
		StartTimer(this, GetWorld(), "EndCurrentPhase", PhaseEndDelay + 0.00001f, false); // 0 does not work o.O
	}
}

void ATDMGameModeBase::SendKillInfoToPlayers(AController * KilledPlayer, AController* KillingPlayer, AActor* DamageCauser) {
	FCharacterInfo killerInfo = KillingPlayer ? KillingPlayer->GetCharacter()->FindComponentByClass<UCharacterInfoProvider>()->GetCharacterInfo() : FCharacterInfo();
	FCharacterInfo killedInfo = KilledPlayer->GetCharacter()->FindComponentByClass<UCharacterInfoProvider>()->GetCharacterInfo();
	DoShitForAllPlayerControllers(GetWorld(), [&](APlayerControllerBase* controller) {
		float killedScoreChange = GetGameState()->CurrentPhase == 0 ? 0 : ScoreHandler->GetKilledScoreChange(KilledPlayer, KillingPlayer);
		float killerScoreChange = GetGameState()->CurrentPhase == 0 ? 0 : ScoreHandler->GetKillerScoreChange(KilledPlayer, KillingPlayer);
		auto weapon = Cast<ABaseWeapon>(DamageCauser);
		controller->ClientShowKillInfo(killedInfo, killedScoreChange, killerInfo, killerScoreChange, weapon ? weapon->WeaponTexture : NULL);
	});
}

void ATDMGameModeBase::SendPhaseStartedToPlayers(int Phase) {
	DoShitForAllPlayerControllers(GetWorld(), [&](APlayerControllerBase* controller) {
		controller->ClientPhaseStarted(Phase);
	});
}

bool ATDMGameModeBase::IsPhaseWinnerFound(AController* controllerToIgnore) {
	return (GetTeamsAlive(controllerToIgnore).Num() == 1);
}

bool ATDMGameModeBase::IsMatchWinnerFound() {
	return GetGameState()->CurrentRound >= GetGameState()->MaxRoundNum
		|| GetGameState()->TeamWins[ScoreHandler->GetWinningTeam()] > GetGameState()->MaxRoundNum / 2;
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

void ATDMGameModeBase::StartReplays() {
	TArray<AActor*> foundActors;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), AReplayCharacterBase::StaticClass(), foundActors);
	for(auto& item : foundActors)
		Cast<AReplayCharacterBase>(item)->StartReplay();
}

void ATDMGameModeBase::SaveRecordingsFromRemainingPlayers() {
	DoShitForAllPlayerControllers(GetWorld(), [&](APlayerControllerBase* controller) {
		if(IsPlayerAlive(controller)) {
			SavePlayerRecordings(controller);
		}
	});
}

void ATDMGameModeBase::SavePlayerRecordings(APlayerControllerBase * playerController) {
	auto playerState = Cast<ATDMPlayerStateBase>(playerController->PlayerState);
	auto pawn = playerController->GetPawn();
	if(AssertNotNull(pawn, GetWorld(), __FILE__, __LINE__) && AssertNotNull(playerState, GetWorld(), __FILE__, __LINE__)) {
		auto playerStateRecorder = pawn->FindComponentByClass<UPlayerStateRecorder>();
		AssertNotNull(playerStateRecorder, GetWorld(), __FILE__, __LINE__);
		if(!PlayerRecordings.Contains(playerController)) {
			TArray<FRecordedPlayerData> list;
			PlayerRecordings.Add(playerController, list);
		}
		FRecordedPlayerData data = FRecordedPlayerData();
		data.EquippedShopItems = playerState->EquippedShopItems;
		data.Phase = GetGameState()->CurrentPhase;
		data.RecordedPlayerStates = playerStateRecorder->RecordedPlayerStates;
		PlayerRecordings[playerController].Add(data);
	}
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

void ATDMGameModeBase::ResetLevel() {
	ATDMLevelScriptBase* levelScript = Cast<ATDMLevelScriptBase>(GetWorld()->GetLevelScriptActor(GetLevel()));
	if(levelScript)
		levelScript->ResetLevel();
}