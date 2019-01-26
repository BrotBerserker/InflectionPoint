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
	: Super() {DebugPrint(__FILE__, __LINE__);
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
DebugPrint(__FILE__, __LINE__);}

void ATDMGameModeBase::EnablePIEQuickPlayMode() {DebugPrint(__FILE__, __LINE__);
	PhaseStartDelay = 1;
	MatchStartDelay = 1;
	ShopTime = 3.0f;
	MatchReStartDelay = 3.0f;
	PhaseEndDelay = 2.0f;
	RoundEndDelay = 3.0f;
DebugPrint(__FILE__, __LINE__);}

void ATDMGameModeBase::PostInitializeComponents() {DebugPrint(__FILE__, __LINE__);
	Super::PostInitializeComponents();
	if(IsPIEQuickPlayModeEnabled && GetWorld()->WorldType == EWorldType::PIE)
		EnablePIEQuickPlayMode();
	MatchStartCountdown->Setup(this, &ATDMGameModeBase::UpdateMatchCountdown, &ATDMGameModeBase::StartMatch, MatchStartDelay);
	ShopCountdown->Setup(this, &ATDMGameModeBase::UpdateShopCountdown, &ATDMGameModeBase::PreparePhaseStart, ShopTime, true);
	PhaseStartCountdown->Setup(this, &ATDMGameModeBase::UpdatePhaseCountdown, &ATDMGameModeBase::StartPhase, PhaseStartDelay);
DebugPrint(__FILE__, __LINE__);}

void ATDMGameModeBase::InitializeSettings(FName SessionName) {DebugPrint(__FILE__, __LINE__);
	IOnlineSessionPtr session = IOnlineSubsystem::Get()->GetSessionInterface();
	FOnlineSessionSettings* sessionSettings = session->GetSessionSettings(SessionName);
	AssertNotNull(GetGameState(), GetWorld(), __FILE__, __LINE__);
	if(sessionSettings) {DebugPrint(__FILE__, __LINE__);
		GetGameState()->MaxPlayers = sessionSettings->NumPublicConnections;
		sessionSettings->Get(FName("Rounds"), GetGameState()->MaxRoundNum);
	DebugPrint(__FILE__, __LINE__);} else {DebugPrint(__FILE__, __LINE__);
		UE_LOG(LogTemp, Warning, TEXT("Warning: No session settings could be found, using offline settings."));
		GetGameState()->MaxPlayers = OfflineMaxPlayers;
		GetGameState()->MaxRoundNum = OfflineMaxRoundNum;
	DebugPrint(__FILE__, __LINE__);}
DebugPrint(__FILE__, __LINE__);}

void ATDMGameModeBase::PreLogin(const FString & Options, const FString & Address, const FUniqueNetIdRepl & UniqueId, FString & ErrorMessage) {DebugPrint(__FILE__, __LINE__);
	Super::PreLogin(Options, Address, UniqueId, ErrorMessage);
	AssertNotNull(GetGameState(), GetWorld(), __FILE__, __LINE__);
	if(GetGameState()->NumPlayers >= GetGameState()->MaxPlayers) {DebugPrint(__FILE__, __LINE__);
		ErrorMessage = "Server is already full!";
	DebugPrint(__FILE__, __LINE__);}
DebugPrint(__FILE__, __LINE__);}

void ATDMGameModeBase::PostLogin(APlayerController * NewPlayer) {DebugPrint(__FILE__, __LINE__);
	Super::PostLogin(NewPlayer);
	AssertNotNull(GetGameState(), GetWorld(), __FILE__, __LINE__);
	GetGameState()->NumPlayers++;
	UpdateCurrentPlayers(Cast<UInflectionPointGameInstanceBase>(GetGameInstance())->CurrentSessionName);
	if(GetGameState()->NumPlayers > GetGameState()->MaxPlayers) {DebugPrint(__FILE__, __LINE__);
		GameSession->KickPlayer(NewPlayer, FText::FromString("Server is already full!"));
		return;
	DebugPrint(__FILE__, __LINE__);}
	if(GetGameState()->NumPlayers == GetGameState()->MaxPlayers) {DebugPrint(__FILE__, __LINE__);
		MatchStartCountdown->Start();
	DebugPrint(__FILE__, __LINE__);}
DebugPrint(__FILE__, __LINE__);}

void ATDMGameModeBase::Logout(AController* Exiting) {DebugPrint(__FILE__, __LINE__);
	Super::Logout(Exiting);
	GetGameState()->NumPlayers--;
	UpdateCurrentPlayers(Cast<UInflectionPointGameInstanceBase>(GetGameInstance())->CurrentSessionName);
	if(GetGameState()->CurrentRound > 0 && IsPhaseWinnerFound(Exiting) && !isPlayingEndMatchSequence) {DebugPrint(__FILE__, __LINE__);
		MatchStartCountdown->Stop();
		PhaseStartCountdown->Stop();
		ShopCountdown->Stop();
		StartEndMatchSequence();
	DebugPrint(__FILE__, __LINE__);}
DebugPrint(__FILE__, __LINE__);}

void ATDMGameModeBase::UpdateCurrentPlayers(FName SessionName) {DebugPrint(__FILE__, __LINE__);
	IOnlineSessionPtr session = IOnlineSubsystem::Get()->GetSessionInterface();
	FOnlineSessionSettings* sessionSettings = session->GetSessionSettings(SessionName);
	if(sessionSettings) {DebugPrint(__FILE__, __LINE__);
		sessionSettings->Set(FName("CurrentPlayers"), GetGameState()->NumPlayers, EOnlineDataAdvertisementType::ViaOnlineServiceAndPing);
		session->UpdateSession(SessionName, *sessionSettings);
	DebugPrint(__FILE__, __LINE__);}
DebugPrint(__FILE__, __LINE__);}

void ATDMGameModeBase::StartMatch() {DebugPrint(__FILE__, __LINE__);
	GetGameState()->PrepareForMatchStart(CharacterSpawner->GetSpawnPointCount());
	CharacterSpawner->AssignTeamsAndPlayerStartGroups();
	StartNextRound();
DebugPrint(__FILE__, __LINE__);}

void ATDMGameModeBase::StartNextRound() {DebugPrint(__FILE__, __LINE__);
	if(!AssertTrue(GetGameState()->CurrentRound < GetGameState()->MaxRoundNum, GetWorld(), __FILE__, __LINE__, "Cant Start next Round"))
		return;
	GetGameState()->PrepareForRoundStart();
	PlayerRecordings.Reset();
	StartNextPhase();
DebugPrint(__FILE__, __LINE__);}

void ATDMGameModeBase::StartNextPhase() {DebugPrint(__FILE__, __LINE__);
	int phase = GetGameState()->CurrentPhase + 1;
	if(!AssertTrue(phase <= GetGameState()->MaxPhaseNum, GetWorld(), __FILE__, __LINE__, "Cant start the next Phase"))
		return;
	GetGameState()->CurrentPhase = phase;
	ResetLevel();
	ShowShops();
	ShopCountdown->Start();
DebugPrint(__FILE__, __LINE__);}

void ATDMGameModeBase::ShowShops() {DebugPrint(__FILE__, __LINE__);
	bool isFirstPhaseInRound = GetGameState()->CurrentPhase == 1;
	DoShitForAllPlayerControllers(GetWorld(), [&](APlayerControllerBase* controller) {DebugPrint(__FILE__, __LINE__);
		controller->ClientShowShop(isFirstPhaseInRound);
	DebugPrint(__FILE__, __LINE__);});
DebugPrint(__FILE__, __LINE__);}

void ATDMGameModeBase::PreparePhaseStart() {DebugPrint(__FILE__, __LINE__);
	CharacterSpawner->SpawnPlayersAndReplays(GetGameState()->CurrentPhase, PlayerRecordings);
	SendPhaseStartedToPlayers(GetGameState()->CurrentPhase);
	PhaseStartCountdown->Start();
	StartTimer(this, GetWorld(), "StartSpawnCinematics", 0.3, false); // needed because rpc not redy ^^
DebugPrint(__FILE__, __LINE__);}

void ATDMGameModeBase::StartPhase() {DebugPrint(__FILE__, __LINE__);
	StartReplays();
	if(IsPhaseWinnerFound()) {DebugPrint(__FILE__, __LINE__);
		StartTimer(this, GetWorld(), "StartEndMatchSequence", 1.1f, false); // wait for countdown animation
	DebugPrint(__FILE__, __LINE__);}
DebugPrint(__FILE__, __LINE__);}

void ATDMGameModeBase::EndCurrentPhase() {DebugPrint(__FILE__, __LINE__);
	SaveRecordingsFromRemainingPlayers();
	if(GetGameState()->CurrentPhase == GetGameState()->MaxPhaseNum) {DebugPrint(__FILE__, __LINE__);
		EndCurrentRound();
	DebugPrint(__FILE__, __LINE__);} else {DebugPrint(__FILE__, __LINE__);
		StartNextPhase();
	DebugPrint(__FILE__, __LINE__);}
DebugPrint(__FILE__, __LINE__);}

void ATDMGameModeBase::EndCurrentRound() {DebugPrint(__FILE__, __LINE__);
	int winnerTeam = ScoreHandler->SelectWinnerTeamForRound();
	ScoreHandler->UpdateScoresForNextRound();
	if(IsMatchWinnerFound()) {DebugPrint(__FILE__, __LINE__);
		StartEndMatchSequence();
		return;
	DebugPrint(__FILE__, __LINE__);}
	NotifyControllersOfEndRound(winnerTeam);
	StartTimer(this, GetWorld(), "StartNextRound", RoundEndDelay, false);
DebugPrint(__FILE__, __LINE__);}

void ATDMGameModeBase::ReStartMatch() {DebugPrint(__FILE__, __LINE__);
	isPlayingEndMatchSequence = false;
	GetGameState()->PrepareForMatchStart(CharacterSpawner->GetSpawnPointCount());
	ResetLevel();
	CharacterSpawner->SpawnAllPlayersForWarmupRound();
	if(GetGameState()->NumPlayers == GetGameState()->MaxPlayers) {DebugPrint(__FILE__, __LINE__);
		MatchStartCountdown->Start();
	DebugPrint(__FILE__, __LINE__);}
DebugPrint(__FILE__, __LINE__);}

void ATDMGameModeBase::UpdateMatchCountdown(int number) {DebugPrint(__FILE__, __LINE__);
	DoShitForAllPlayerControllers(GetWorld(), [&](APlayerControllerBase* controller) {DebugPrint(__FILE__, __LINE__);
		controller->ClientShowMatchCountdownNumber(number);
	DebugPrint(__FILE__, __LINE__);});
DebugPrint(__FILE__, __LINE__);}

void ATDMGameModeBase::UpdatePhaseCountdown(int number) {DebugPrint(__FILE__, __LINE__);
	DoShitForAllPlayerControllers(GetWorld(), [&](APlayerControllerBase* controller) {DebugPrint(__FILE__, __LINE__);
		controller->ClientShowPhaseCountdownNumber(number);
	DebugPrint(__FILE__, __LINE__);});
DebugPrint(__FILE__, __LINE__);}

void ATDMGameModeBase::UpdateShopCountdown(int number) {DebugPrint(__FILE__, __LINE__);
	DoShitForAllPlayerControllers(GetWorld(), [&](APlayerControllerBase* controller) {DebugPrint(__FILE__, __LINE__);
		controller->ClientShowShopCountdownNumber(number);
	DebugPrint(__FILE__, __LINE__);});
DebugPrint(__FILE__, __LINE__);}

void ATDMGameModeBase::StartEndMatchSequence() {DebugPrint(__FILE__, __LINE__);
	GetGameState()->ResetPlayerScores(); // so all players appear alive
	ScoreHandler->SetCurrentScoresToTotalScore();
	isPlayingEndMatchSequence = true;
	int winningTeam = ScoreHandler->GetWinningTeam();
	int losingTeam = ScoreHandler->GetLosingTeam();
	NotifyControllersOfEndMatch(winningTeam);
	StartTimer(this, GetWorld(), "ReStartMatch", MatchReStartDelay, false);

	// if no levelscript is provided, just restart the match without playing an end match sequence
	ATDMLevelScriptBase* levelScript = Cast<ATDMLevelScriptBase>(GetWorld()->GetLevelScriptActor(GetLevel()));
	if(!levelScript) {DebugPrint(__FILE__, __LINE__);
		return;
	DebugPrint(__FILE__, __LINE__);}
	ResetLevel();
	FString winnerName = GetAnyPlayerControllerInTeam(winningTeam) ? GetAnyPlayerControllerInTeam(winningTeam)->PlayerState->GetPlayerName() : "Winnerboi";
	FString loserName = GetAnyPlayerControllerInTeam(losingTeam) ? GetAnyPlayerControllerInTeam(losingTeam)->PlayerState->GetPlayerName() : "Kacknoob";
	levelScript->StartEndMatchSequence(CharacterSpawner->PlayerCharacters[winningTeam], CharacterSpawner->PlayerCharacters[losingTeam], winnerName, loserName);
DebugPrint(__FILE__, __LINE__);}

void ATDMGameModeBase::NotifyControllersOfEndMatch(int winnerTeam) {DebugPrint(__FILE__, __LINE__);
	DoShitForAllPlayerControllers(GetWorld(), [&](APlayerControllerBase* controller) {DebugPrint(__FILE__, __LINE__);
		controller->ClientShowMatchEnd(winnerTeam);
	DebugPrint(__FILE__, __LINE__);});
DebugPrint(__FILE__, __LINE__);}

void ATDMGameModeBase::NotifyControllersOfEndRound(int winnerTeam) {DebugPrint(__FILE__, __LINE__);
	DoShitForAllPlayerControllers(GetWorld(), [&](APlayerControllerBase* controller) {DebugPrint(__FILE__, __LINE__);
		controller->ClientShowRoundEnd(winnerTeam);
	DebugPrint(__FILE__, __LINE__);});
DebugPrint(__FILE__, __LINE__);}

void ATDMGameModeBase::StartSpawnCinematics() {DebugPrint(__FILE__, __LINE__);
	ATDMLevelScriptBase* levelScript = Cast<ATDMLevelScriptBase>(GetWorld()->GetLevelScriptActor(GetLevel()));
	if(!levelScript) {DebugPrint(__FILE__, __LINE__);
		return;
	DebugPrint(__FILE__, __LINE__);}
	levelScript->MulticastStartSpawnCinematic();
DebugPrint(__FILE__, __LINE__);}

void ATDMGameModeBase::CharacterDied(AController * KilledPlayer, AController* KillingPlayer, AActor* DamageCauser) {DebugPrint(__FILE__, __LINE__);
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

	if(GetGameState()->CurrentPhase == 0) {DebugPrint(__FILE__, __LINE__);
		CharacterSpawner->SpawnAndPossessPlayer(playerController, 0);
	DebugPrint(__FILE__, __LINE__);} else if(IsPhaseWinnerFound()) {DebugPrint(__FILE__, __LINE__);
		StartTimer(this, GetWorld(), "EndCurrentPhase", PhaseEndDelay + 0.00001f, false); // 0 does not work o.O
	DebugPrint(__FILE__, __LINE__);}
DebugPrint(__FILE__, __LINE__);}

void ATDMGameModeBase::SendKillInfoToPlayers(AController * KilledPlayer, AController* KillingPlayer, AActor* DamageCauser) {DebugPrint(__FILE__, __LINE__);
	FCharacterInfo killerInfo = KillingPlayer ? KillingPlayer->GetCharacter()->FindComponentByClass<UCharacterInfoProvider>()->GetCharacterInfo() : FCharacterInfo();
	FCharacterInfo killedInfo = KilledPlayer->GetCharacter()->FindComponentByClass<UCharacterInfoProvider>()->GetCharacterInfo();
	DoShitForAllPlayerControllers(GetWorld(), [&](APlayerControllerBase* controller) {DebugPrint(__FILE__, __LINE__);
		float killedScoreChange = GetGameState()->CurrentPhase == 0 ? 0 : ScoreHandler->GetKilledScoreChange(KilledPlayer, KillingPlayer);
		float killerScoreChange = GetGameState()->CurrentPhase == 0 ? 0 : ScoreHandler->GetKillerScoreChange(KilledPlayer, KillingPlayer);
		auto weapon = Cast<ABaseWeapon>(DamageCauser);
		controller->ClientShowKillInfo(killedInfo, killedScoreChange, killerInfo, killerScoreChange, weapon ? weapon->WeaponTexture : NULL);
	DebugPrint(__FILE__, __LINE__);});
DebugPrint(__FILE__, __LINE__);}

void ATDMGameModeBase::SendPhaseStartedToPlayers(int Phase) {DebugPrint(__FILE__, __LINE__);
	DoShitForAllPlayerControllers(GetWorld(), [&](APlayerControllerBase* controller) {DebugPrint(__FILE__, __LINE__);
		controller->ClientPhaseStarted(Phase);
	DebugPrint(__FILE__, __LINE__);});
DebugPrint(__FILE__, __LINE__);}

bool ATDMGameModeBase::IsPhaseWinnerFound(AController* controllerToIgnore) {DebugPrint(__FILE__, __LINE__);
	return (GetTeamsAlive(controllerToIgnore).Num() == 1);
DebugPrint(__FILE__, __LINE__);}

bool ATDMGameModeBase::IsMatchWinnerFound() {DebugPrint(__FILE__, __LINE__);
	return GetGameState()->CurrentRound >= GetGameState()->MaxRoundNum
		|| GetGameState()->TeamWins[ScoreHandler->GetWinningTeam()] > GetGameState()->MaxRoundNum / 2;
DebugPrint(__FILE__, __LINE__);}

TArray<int> ATDMGameModeBase::GetTeamsAlive(AController* controllerToIgnore) {DebugPrint(__FILE__, __LINE__);
	TArray<int> teamsAlive;
	for(FConstPlayerControllerIterator Iterator = GetWorld()->GetPlayerControllerIterator(); Iterator; ++Iterator) {DebugPrint(__FILE__, __LINE__);
		auto playerController = UGameplayStatics::GetPlayerController(GetWorld(), Iterator.GetIndex());
		auto ipPlayerController = Cast<APlayerControllerBase>(playerController);
		auto playerState = Cast<ATDMPlayerStateBase>(playerController->PlayerState);

		if(ipPlayerController == controllerToIgnore || teamsAlive.Contains(playerState->Team))
			continue;
		if(IsPlayerAlive(ipPlayerController))
			teamsAlive.Add(playerState->Team);
	DebugPrint(__FILE__, __LINE__);}
	return teamsAlive;
DebugPrint(__FILE__, __LINE__);}

bool ATDMGameModeBase::IsPlayerAlive(APlayerControllerBase* playerController) {DebugPrint(__FILE__, __LINE__);
	return Cast<ATDMPlayerStateBase>(playerController->PlayerState)->IsAlive;
DebugPrint(__FILE__, __LINE__);}

void ATDMGameModeBase::StartReplays() {DebugPrint(__FILE__, __LINE__);
	TArray<AActor*> foundActors;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), AReplayCharacterBase::StaticClass(), foundActors);
	for(auto& item : foundActors)
		Cast<AReplayCharacterBase>(item)->StartReplay();
DebugPrint(__FILE__, __LINE__);}

void ATDMGameModeBase::SaveRecordingsFromRemainingPlayers() {DebugPrint(__FILE__, __LINE__);
	DoShitForAllPlayerControllers(GetWorld(), [&](APlayerControllerBase* controller) {DebugPrint(__FILE__, __LINE__);
		if(IsPlayerAlive(controller)) {DebugPrint(__FILE__, __LINE__);
			SavePlayerRecordings(controller);
		DebugPrint(__FILE__, __LINE__);}
	DebugPrint(__FILE__, __LINE__);});
DebugPrint(__FILE__, __LINE__);}

void ATDMGameModeBase::SavePlayerRecordings(APlayerControllerBase * playerController) {DebugPrint(__FILE__, __LINE__);
	auto playerState = Cast<ATDMPlayerStateBase>(playerController->PlayerState);
	auto pawn = playerController->GetPawn();
	if(AssertNotNull(pawn, GetWorld(), __FILE__, __LINE__) && AssertNotNull(playerState, GetWorld(), __FILE__, __LINE__)) {DebugPrint(__FILE__, __LINE__);
		auto playerStateRecorder = pawn->FindComponentByClass<UPlayerStateRecorder>();
		AssertNotNull(playerStateRecorder, GetWorld(), __FILE__, __LINE__);
		if(!PlayerRecordings.Contains(playerController)) {DebugPrint(__FILE__, __LINE__);
			TArray<FRecordedPlayerData> list;
			PlayerRecordings.Add(playerController, list);
		DebugPrint(__FILE__, __LINE__);}
		FRecordedPlayerData data = FRecordedPlayerData();
		data.EquippedShopItems = playerState->EquippedShopItems;
		data.Phase = GetGameState()->CurrentPhase;
		data.RecordedPlayerStates = playerStateRecorder->RecordedPlayerStates;
		PlayerRecordings[playerController].Add(data);
	DebugPrint(__FILE__, __LINE__);}
DebugPrint(__FILE__, __LINE__);}

APlayerController* ATDMGameModeBase::GetAnyPlayerControllerInTeam(int team) {DebugPrint(__FILE__, __LINE__);
	for(FConstPlayerControllerIterator Iterator = GetWorld()->GetPlayerControllerIterator(); Iterator; ++Iterator) {DebugPrint(__FILE__, __LINE__);
		auto playerController = UGameplayStatics::GetPlayerController(GetWorld(), Iterator.GetIndex());
		ATDMPlayerStateBase* playerState = Cast<ATDMPlayerStateBase>(playerController->PlayerState);
		if(playerState && playerState->Team == team) {DebugPrint(__FILE__, __LINE__);
			return playerController;
		DebugPrint(__FILE__, __LINE__);}
	DebugPrint(__FILE__, __LINE__);}
	return nullptr;
DebugPrint(__FILE__, __LINE__);}

void ATDMGameModeBase::ResetLevel() {DebugPrint(__FILE__, __LINE__);
	ATDMLevelScriptBase* levelScript = Cast<ATDMLevelScriptBase>(GetWorld()->GetLevelScriptActor(GetLevel()));
	if(levelScript)
		levelScript->ResetLevel();
DebugPrint(__FILE__, __LINE__);}