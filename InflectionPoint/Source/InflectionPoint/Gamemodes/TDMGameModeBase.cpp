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
DebugPrint(__FILE__, __LINE__);}

void ATDMGameModeBase::PostLogin(APlayerController * NewPlayer) {DebugPrint(__FILE__, __LINE__);
	Super::PostLogin(NewPlayer);
	if(NumPlayers >= MaxPlayers) {DebugPrint(__FILE__, __LINE__);
		GameSession->KickPlayer(NewPlayer, FText::FromString("Server is already full!"));
		return;
	DebugPrint(__FILE__, __LINE__);}
	NumPlayers++;
	UpdateCurrentPlayers(Cast<UInflectionPointGameInstanceBase>(GetGameInstance())->CurrentSessionName);
	APlayerControllerBase* controller = Cast<APlayerControllerBase>(NewPlayer);
	if(NumPlayers == MaxPlayers)
		StartMatch();
DebugPrint(__FILE__, __LINE__);}

void ATDMGameModeBase::PreLogin(const FString & Options, const FString & Address, const FUniqueNetIdRepl & UniqueId, FString & ErrorMessage) {DebugPrint(__FILE__, __LINE__);
	Super::PreLogin(Options, Address, UniqueId, ErrorMessage);
	if(NumPlayers >= MaxPlayers) {DebugPrint(__FILE__, __LINE__);
		ErrorMessage = "Server is already full!";
	DebugPrint(__FILE__, __LINE__);}
DebugPrint(__FILE__, __LINE__);}

void ATDMGameModeBase::UpdateMaxPlayers(FName SessionName) {DebugPrint(__FILE__, __LINE__);
	IOnlineSessionPtr session = IOnlineSubsystem::Get()->GetSessionInterface();
	FOnlineSessionSettings* sessionSettings = session->GetSessionSettings(SessionName);
	if(sessionSettings) {DebugPrint(__FILE__, __LINE__);
		MaxPlayers = sessionSettings->NumPublicConnections;
	DebugPrint(__FILE__, __LINE__);} else {DebugPrint(__FILE__, __LINE__);
		UE_LOG(LogTemp, Warning, TEXT("Warning: No session settings could be found, setting MaxPlayers to %d."), OfflineMaxPlayers);
		MaxPlayers = OfflineMaxPlayers;
	DebugPrint(__FILE__, __LINE__);}
DebugPrint(__FILE__, __LINE__);}

void ATDMGameModeBase::UpdateCurrentPlayers(FName SessionName) {DebugPrint(__FILE__, __LINE__);
	IOnlineSessionPtr session = IOnlineSubsystem::Get()->GetSessionInterface();
	FOnlineSessionSettings* sessionSettings = session->GetSessionSettings(SessionName);
	if(sessionSettings) {DebugPrint(__FILE__, __LINE__);
		sessionSettings->Set(FName("CurrentPlayers"), NumPlayers, EOnlineDataAdvertisementType::ViaOnlineServiceAndPing);
		session->UpdateSession(SessionName, *sessionSettings);
	DebugPrint(__FILE__, __LINE__);} 
DebugPrint(__FILE__, __LINE__);}

void ATDMGameModeBase::StartMatch() {DebugPrint(__FILE__, __LINE__);
	GetGameState()->CurrentRound = 0;
	GetGameState()->MaxRoundNum = CountSpawnPoints() / OfflineMaxPlayers;
	AssignTeamsAndPlayerStartGroups();
	ResetPlayerScores();
	StartTimer(this, GetWorld(), "StartNextRound", MatchStartDelay + 0.00001f, false); // we can't call "StartMatch" with a timer because that way the teams will not be replicated to the client before the characters are spawned 
DebugPrint(__FILE__, __LINE__);}

void ATDMGameModeBase::EndCurrentRound() {DebugPrint(__FILE__, __LINE__);
	SaveRecordingsFromRemainingPlayers();
	StartNextRound();
DebugPrint(__FILE__, __LINE__);}

void ATDMGameModeBase::StartNextRound() {DebugPrint(__FILE__, __LINE__);
	int round = GetGameState()->CurrentRound + 1;
	if(round > GetGameState()->MaxRoundNum)
		round = 1; // restart 
	GetGameState()->CurrentRound = round;
	ClearMap();
	SpawnPlayersAndReplays();
	SendRoundStartedToPlayers(round);
	StartCountdown();
	StartTimer(this, GetWorld(), "StartSpawnCinematics", 0.3, false); // needed because rpc not redy ^^
DebugPrint(__FILE__, __LINE__);}

void ATDMGameModeBase::StartSpawnCinematics() {DebugPrint(__FILE__, __LINE__);
	ATDMLevelScriptBase* levelScript = Cast<ATDMLevelScriptBase>(GetWorld()->GetLevelScriptActor(GetLevel()));
	if(!levelScript) {DebugPrint(__FILE__, __LINE__);
		SoftAssertTrue(false, GetWorld(), __FILE__, __LINE__, "Could not play SpawnCinematic");
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

	if(GetGameState()->CurrentRound > 0)
		ScoreHandler->AddKill(KilledPlayer, KillingPlayer);

	APlayerControllerBase* playerController = Cast<APlayerControllerBase>(KilledPlayer);
	if(!playerController)
		return;

	if(GetGameState()->CurrentRound > 0)
		SavePlayerRecordings(playerController);

	if(GetGameState()->CurrentRound == 0) {DebugPrint(__FILE__, __LINE__);
		SpawnAndPossessPlayer(playerController);
	DebugPrint(__FILE__, __LINE__);} else if(IsWinnerFound()) {DebugPrint(__FILE__, __LINE__);
		StartTimer(this, GetWorld(), "EndCurrentRound", RoundEndDelay + 0.00001f, false); // 0 does not work o.O
	DebugPrint(__FILE__, __LINE__);}
DebugPrint(__FILE__, __LINE__);}


void ATDMGameModeBase::ResetPlayerScores() {DebugPrint(__FILE__, __LINE__);
	for(FConstPlayerControllerIterator Iterator = GetWorld()->GetPlayerControllerIterator(); Iterator; ++Iterator) {DebugPrint(__FILE__, __LINE__);
		auto playerController = UGameplayStatics::GetPlayerController(GetWorld(), Iterator.GetIndex());
		Cast<ATDMPlayerStateBase>(playerController->PlayerState)->ResetScore();
	DebugPrint(__FILE__, __LINE__);}
DebugPrint(__FILE__, __LINE__);}

void ATDMGameModeBase::SendKillInfoToPlayers(AController * KilledPlayer, AController* KillingPlayer, AActor* DamageCauser) {DebugPrint(__FILE__, __LINE__);
	FCharacterInfo killerInfo = KillingPlayer ? KillingPlayer->GetCharacter()->FindComponentByClass<UCharacterInfoProvider>()->GetCharacterInfo() : FCharacterInfo();
	FCharacterInfo killedInfo = KilledPlayer->GetCharacter()->FindComponentByClass<UCharacterInfoProvider>()->GetCharacterInfo();
	for(FConstPlayerControllerIterator Iterator = GetWorld()->GetPlayerControllerIterator(); Iterator; ++Iterator) {DebugPrint(__FILE__, __LINE__);
		auto playerController = UGameplayStatics::GetPlayerController(GetWorld(), Iterator.GetIndex());
		APlayerControllerBase* controller = Cast<APlayerControllerBase>(playerController);
		float killedScoreChange = GetGameState()->CurrentRound == 0 ? 0 : ScoreHandler->GetKilledScoreChange(KilledPlayer, KillingPlayer);
		float killerScoreChange = GetGameState()->CurrentRound == 0 ? 0 : ScoreHandler->GetKillerScoreChange(KilledPlayer, KillingPlayer);
		controller->ClientShowKillInfo(killedInfo, killedScoreChange, killerInfo, killerScoreChange, NULL);
	DebugPrint(__FILE__, __LINE__);}
DebugPrint(__FILE__, __LINE__);}

void ATDMGameModeBase::SendRoundStartedToPlayers(int Round) {DebugPrint(__FILE__, __LINE__);
	for(FConstPlayerControllerIterator Iterator = GetWorld()->GetPlayerControllerIterator(); Iterator; ++Iterator) {DebugPrint(__FILE__, __LINE__);
		auto playerController = UGameplayStatics::GetPlayerController(GetWorld(), Iterator.GetIndex());
		APlayerControllerBase* controller = Cast<APlayerControllerBase>(playerController);
		controller->ClientRoundStarted(Round);
	DebugPrint(__FILE__, __LINE__);}
DebugPrint(__FILE__, __LINE__);}

bool ATDMGameModeBase::IsWinnerFound() {DebugPrint(__FILE__, __LINE__);
	return (GetTeamsAlive().Num() == 1);
DebugPrint(__FILE__, __LINE__);}

TArray<int> ATDMGameModeBase::GetTeamsAlive() {DebugPrint(__FILE__, __LINE__);
	TArray<int> teamsAlive;
	for(FConstPlayerControllerIterator Iterator = GetWorld()->GetPlayerControllerIterator(); Iterator; ++Iterator) {DebugPrint(__FILE__, __LINE__);
		auto playerController = UGameplayStatics::GetPlayerController(GetWorld(), Iterator.GetIndex());
		auto ipPlayerController = Cast<APlayerControllerBase>(playerController);
		auto playerState = Cast<ATDMPlayerStateBase>(playerController->PlayerState);

		if(teamsAlive.Contains(playerState->Team))
			continue;
		if(IsPlayerAlive(ipPlayerController))
			teamsAlive.Add(playerState->Team);
	DebugPrint(__FILE__, __LINE__);}
	return teamsAlive;
DebugPrint(__FILE__, __LINE__);}


bool ATDMGameModeBase::IsPlayerAlive(APlayerControllerBase* playerController) {DebugPrint(__FILE__, __LINE__);
	return Cast<ATDMPlayerStateBase>(playerController->PlayerState)->IsAlive;
DebugPrint(__FILE__, __LINE__);}

void ATDMGameModeBase::SpawnPlayersAndReplays() {DebugPrint(__FILE__, __LINE__);
	for(FConstPlayerControllerIterator Iterator = GetWorld()->GetPlayerControllerIterator(); Iterator; ++Iterator) {DebugPrint(__FILE__, __LINE__);
		auto playerController = UGameplayStatics::GetPlayerController(GetWorld(), Iterator.GetIndex());
		auto ipPlayerController = Cast<APlayerControllerBase>(playerController);
		SpawnAndPossessPlayer(ipPlayerController);
		for(int i = 1; i < GetGameState()->CurrentRound; i++)
			SpawnAndPrepareReplay(ipPlayerController, i);
	DebugPrint(__FILE__, __LINE__);}
DebugPrint(__FILE__, __LINE__);}

void ATDMGameModeBase::StartCountdown() {DebugPrint(__FILE__, __LINE__);
	TArray<AActor*> foundActors;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), APlayerControllerBase::StaticClass(), foundActors);
	for(auto& controller : foundActors) {DebugPrint(__FILE__, __LINE__);
		APlayerControllerBase* playerController = Cast<APlayerControllerBase>(controller);
		//playerController->ClientSetIgnoreInput(true); TODO
	DebugPrint(__FILE__, __LINE__);}
	for(int i = CountDownDuration; i >= 0; i--) {DebugPrint(__FILE__, __LINE__);
		StartTimer(this, GetWorld(), "UpdateCountdown", (CountDownDuration - i + 1), false, foundActors, i);
	DebugPrint(__FILE__, __LINE__);}
DebugPrint(__FILE__, __LINE__);}

void ATDMGameModeBase::UpdateCountdown(TArray<AActor*> controllers, int number) {DebugPrint(__FILE__, __LINE__);
	for(auto& controller : controllers) {DebugPrint(__FILE__, __LINE__);
		APlayerControllerBase* playerController = Cast<APlayerControllerBase>(controller);
		playerController->ClientShowCountdownNumber(number);
		if(number == 0) {DebugPrint(__FILE__, __LINE__);
			playerController->GetCharacter()->FindComponentByClass<UPlayerStateRecorder>()->ServerStartRecording();
			playerController->ClientSetIgnoreInput(false);
		DebugPrint(__FILE__, __LINE__);}
	DebugPrint(__FILE__, __LINE__);}
	if(number == 0) {DebugPrint(__FILE__, __LINE__);
		StartReplays();
	DebugPrint(__FILE__, __LINE__);}
DebugPrint(__FILE__, __LINE__);}

template <typename CharacterType>
CharacterType* ATDMGameModeBase::SpawnCharacter(UClass* spawnClass, APlayerControllerBase * playerController, AActor* playerStart) {DebugPrint(__FILE__, __LINE__);
	FVector loc = playerStart->GetTransform().GetLocation();
	FRotator rot = FRotator(playerStart->GetTransform().GetRotation());

	FActorSpawnParameters ActorSpawnParams;
	ActorSpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	CharacterType* newCharacter = GetWorld()->SpawnActor<CharacterType>(spawnClass, loc, rot, ActorSpawnParams);
	AssertNotNull(newCharacter, GetWorld(), __FILE__, __LINE__, "Could not spawn character!");
	return newCharacter;
DebugPrint(__FILE__, __LINE__);}

void ATDMGameModeBase::SpawnAndPossessPlayer(APlayerControllerBase * playerController) {DebugPrint(__FILE__, __LINE__);
	AActor* spawnPoint = FindSpawnForPlayer(playerController, GetGameState()->CurrentRound);
	AssertNotNull(spawnPoint, GetWorld(), __FILE__, __LINE__);

	auto character = SpawnCharacter<APlayerCharacterBase>(DefaultPawnClass.Get(), playerController, spawnPoint);
	AssertNotNull(character, GetWorld(), __FILE__, __LINE__);

	playerController->ClientSetControlRotation(FRotator(spawnPoint->GetTransform().GetRotation()));
	playerController->Possess(character);
	Cast<ATDMPlayerStateBase>(playerController->PlayerState)->IsAlive = true;
DebugPrint(__FILE__, __LINE__);}

void ATDMGameModeBase::SpawnAndPrepareReplay(APlayerControllerBase* playerController, int round) {DebugPrint(__FILE__, __LINE__);
	AssertTrue(PlayerRecordings.Contains(playerController), GetWorld(), __FILE__, __LINE__, "Could not find replay");
	auto spawnPoint = FindSpawnForPlayer(playerController, round);
	AssertNotNull(spawnPoint, GetWorld(), __FILE__, __LINE__, "No spawn found");
	auto character = SpawnCharacter<AReplayCharacterBase>(ReplayCharacter, playerController, spawnPoint);

	AssertNotNull(character, GetWorld(), __FILE__, __LINE__);
	AssertTrue(PlayerRecordings[playerController].Contains(round), GetWorld(), __FILE__, __LINE__, "Could not find replay");

	// Start Replay on spawned ReplayCharacter
	if(PlayerRecordings[playerController].Contains(round))
		character->SetReplayData(PlayerRecordings[playerController][round]);

	Cast<AAIControllerBase>(character->GetController())->Initialize(playerController);
DebugPrint(__FILE__, __LINE__);}

void ATDMGameModeBase::StartReplays() {DebugPrint(__FILE__, __LINE__);
	TArray<AActor*> foundActors;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), AReplayCharacterBase::StaticClass(), foundActors);
	for(auto& item : foundActors)
		Cast<AReplayCharacterBase>(item)->StartReplay();
DebugPrint(__FILE__, __LINE__);}

AActor* ATDMGameModeBase::FindSpawnForPlayer(APlayerControllerBase * playerController, int round) {DebugPrint(__FILE__, __LINE__);
	if(round == 0)
		return FindPlayerStart(playerController);
	return FindPlayerStart(playerController, GetSpawnTag(playerController, round));
DebugPrint(__FILE__, __LINE__);}

FString ATDMGameModeBase::GetSpawnTag(APlayerControllerBase*  playerController, int round) {DebugPrint(__FILE__, __LINE__);
	auto playerState = Cast<ATDMPlayerStateBase>(playerController->PlayerState);
	int teams = 2;
	int PlayerPerTeam = (OfflineMaxPlayers / 2);
	int spawnsPerTeam = CountSpawnPoints() / teams;
	int spawnIndex = ((playerState->PlayerStartGroup) * (spawnsPerTeam / PlayerPerTeam)) + (round);
	FString spawnTag = FString::FromInt(playerState->Team) + "|" + FString::FromInt(spawnIndex);
	return spawnTag;
DebugPrint(__FILE__, __LINE__);}

int ATDMGameModeBase::CountSpawnPoints() {
	TArray<AActor*> foundActors;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), APlayerStart::StaticClass(), foundActors);
	return foundActors.Num();
}

void ATDMGameModeBase::AssignTeamsAndPlayerStartGroups() {
	UWorld* world = GetWorld();

	for(auto iterator = world->GetPlayerControllerIterator(); iterator; ++iterator) {DebugPrint(__FILE__, __LINE__);
		APlayerControllerBase* controller = (APlayerControllerBase*)UGameplayStatics::GetPlayerController(world, iterator.GetIndex());
		ATDMPlayerStateBase* playerState = Cast<ATDMPlayerStateBase>(controller->PlayerState);
		AssertNotNull(playerState, GetWorld(), __FILE__, __LINE__);
		playerState->Team = iterator.GetIndex() % 2 + 1;
		playerState->PlayerStartGroup = iterator.GetIndex() / 2;
	}
}

void ATDMGameModeBase::SaveRecordingsFromRemainingPlayers() {DebugPrint(__FILE__, __LINE__);
	for(FConstPlayerControllerIterator Iterator = GetWorld()->GetPlayerControllerIterator(); Iterator; ++Iterator) {DebugPrint(__FILE__, __LINE__);
		auto playerController = UGameplayStatics::GetPlayerController(GetWorld(), Iterator.GetIndex());
		auto ipPlayerController = Cast<APlayerControllerBase>(playerController);
		if(IsPlayerAlive(ipPlayerController))
			SavePlayerRecordings(ipPlayerController);
	DebugPrint(__FILE__, __LINE__);}
DebugPrint(__FILE__, __LINE__);}

void ATDMGameModeBase::SavePlayerRecordings(APlayerControllerBase * playerController) {DebugPrint(__FILE__, __LINE__);
	auto pawn = playerController->GetPawn();
	if(AssertNotNull(pawn, GetWorld(), __FILE__, __LINE__)) {DebugPrint(__FILE__, __LINE__);
		auto playerStateRecorder = pawn->FindComponentByClass<UPlayerStateRecorder>();
		AssertNotNull(playerStateRecorder, GetWorld(), __FILE__, __LINE__);
		if(!PlayerRecordings.Contains(playerController)) {DebugPrint(__FILE__, __LINE__);
			TMap<int, TArray<FRecordedPlayerState>> map;
			PlayerRecordings.Add(playerController, map);
		DebugPrint(__FILE__, __LINE__);}
		PlayerRecordings[playerController].Add(GetGameState()->CurrentRound, playerStateRecorder->RecordedPlayerStates);
	DebugPrint(__FILE__, __LINE__);}
DebugPrint(__FILE__, __LINE__);}

void ATDMGameModeBase::ClearMap() {DebugPrint(__FILE__, __LINE__);
	DestroyAllActors(AReplayCharacterBase::StaticClass());
	DestroyAllActors(APlayerCharacterBase::StaticClass());
	DestroyAllActors(AInflectionPointProjectile::StaticClass());
DebugPrint(__FILE__, __LINE__);}

void ATDMGameModeBase::DestroyAllActors(TSubclassOf<AActor> actorClass) {DebugPrint(__FILE__, __LINE__);
	TArray<AActor*> foundActors;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), actorClass, foundActors);
	for(auto& item : foundActors)
		item->Destroy();
DebugPrint(__FILE__, __LINE__);}