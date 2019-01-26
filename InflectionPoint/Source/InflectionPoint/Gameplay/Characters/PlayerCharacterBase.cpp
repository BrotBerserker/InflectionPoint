// Fill out your copyright notice in the Description page of Project Settings.

#include "InflectionPoint.h"
#include "PlayerCharacterBase.h"
#include "Gameplay/Characters/ReplayCharacterBase.h"
#include "Gameplay/Weapons/InflectionPointProjectile.h"
#include "Gameplay/Controllers/AIControllerBase.h"
#include "Utils/CheckFunctions.h"

bool APlayerCharacterBase::IsReadyForInitialization() {DebugPrint(__FILE__, __LINE__);
	if(!GetPlayerState()) {DebugPrint(__FILE__, __LINE__);
		return false;
	DebugPrint(__FILE__, __LINE__);}
	return true;
DebugPrint(__FILE__, __LINE__);}

void APlayerCharacterBase::Initialize() {DebugPrint(__FILE__, __LINE__);
	Super::Initialize();
DebugPrint(__FILE__, __LINE__);}


//////////////////////////////////////////////////////////////////////////
// Input

void APlayerCharacterBase::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) {DebugPrint(__FILE__, __LINE__);
	// set up gameplay key bindings
	check(PlayerInputComponent);

	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ACharacter::Jump);
	PlayerInputComponent->BindAction("Jump", IE_Released, this, &ACharacter::StopJumping);

	PlayerInputComponent->BindAction("Fire", IE_Pressed, this, &ABaseCharacter::StartFire);
	PlayerInputComponent->BindAction("Fire", IE_Released, this, &ABaseCharacter::StopFire);

	PlayerInputComponent->BindAction("Aim", IE_Pressed, this, &ABaseCharacter::StartAiming);
	PlayerInputComponent->BindAction("Aim", IE_Released, this, &ABaseCharacter::StopAiming);

	PlayerInputComponent->BindAction("Reload", IE_Pressed, this, &ABaseCharacter::ServerReload);

	PlayerInputComponent->BindAction("NextWeapon", IE_Pressed, this, &ABaseCharacter::ServerEquipNextWeapon);
	PlayerInputComponent->BindAction("PreviousWeapon", IE_Pressed, this, &ABaseCharacter::ServerEquipPreviousWeapon);

	PlayerInputComponent->BindAction("SwitchToWeapon1", IE_Pressed, this, &APlayerCharacterBase::EquipSpecificWeapon<EInventorySlotPosition::Weapon1>);
	PlayerInputComponent->BindAction("SwitchToWeapon2", IE_Pressed, this, &APlayerCharacterBase::EquipSpecificWeapon<EInventorySlotPosition::Weapon2>);
	PlayerInputComponent->BindAction("SwitchToWeapon3", IE_Pressed, this, &APlayerCharacterBase::EquipSpecificWeapon<EInventorySlotPosition::Weapon3>);
	//PlayerInputComponent->BindAction("SwitchToWeapon4", IE_Pressed, this, &APlayerCharacterBase::EquipSpecificWeapon<3>);
	//PlayerInputComponent->BindAction("SwitchToWeapon5", IE_Pressed, this, &APlayerCharacterBase::EquipSpecificWeapon<4>);
	//PlayerInputComponent->BindAction("SwitchToWeapon6", IE_Pressed, this, &APlayerCharacterBase::EquipSpecificWeapon<5>);

	PlayerInputComponent->BindAction("Sprint", IE_Pressed, this, &ABaseCharacter::EnableSprint);
	PlayerInputComponent->BindAction("Sprint", IE_Released, this, &ABaseCharacter::DisableSprint);

	PlayerInputComponent->BindAxis("MoveForward", this, &ABaseCharacter::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &ABaseCharacter::MoveRight);

	PlayerInputComponent->BindAxis("Turn", this, &APlayerCharacterBase::TurnAtRate);
	PlayerInputComponent->BindAxis("LookUp", this, &APlayerCharacterBase::LookUpAtRate);

	// DEBUG Bindings
	PlayerInputComponent->BindAction("DEBUG_SpawnReplay", IE_Pressed, this, &APlayerCharacterBase::DEBUG_ServerSpawnReplay);
	PlayerInputComponent->BindAction("DEBUG_StartRecording", IE_Pressed, this, &APlayerCharacterBase::DEBUG_StartRecording);

	// Initialize and start PlayerStateRecorder
	PlayerStateRecorder = FindComponentByClass<UPlayerStateRecorder>();
	AssertNotNull(PlayerStateRecorder, GetWorld(), __FILE__, __LINE__);
	PlayerStateRecorder->InitializeBindings(PlayerInputComponent);
DebugPrint(__FILE__, __LINE__);}

void APlayerCharacterBase::Tick(float DeltaTime) {DebugPrint(__FILE__, __LINE__);
	Super::Tick(DeltaTime);
	UpdateMesh3PRenderCustomDepth();
DebugPrint(__FILE__, __LINE__);}

void APlayerCharacterBase::UpdateMesh3PRenderCustomDepth() {DebugPrint(__FILE__, __LINE__);
	auto controller = GetWorld()->GetFirstPlayerController();
	if(controller) {DebugPrint(__FILE__, __LINE__);
		bool canSeeCharacter = controller->LineOfSightTo(this);
		auto playerState = Cast<ATDMPlayerStateBase>(controller->PlayerState);
		bool isInSameTeam = playerState ? playerState->Team == CharacterInfoProvider->GetCharacterInfo().Team : false;
		Mesh3P->SetRenderCustomDepth(!isInSameTeam || !canSeeCharacter);
		Mesh3P->SetCustomDepthStencilValue(isInSameTeam ? 10 : 20);
	DebugPrint(__FILE__, __LINE__);}
DebugPrint(__FILE__, __LINE__);}

template<EInventorySlotPosition slot>
void APlayerCharacterBase::EquipSpecificWeapon() {DebugPrint(__FILE__, __LINE__);
	ServerEquipSpecificWeapon(slot);
DebugPrint(__FILE__, __LINE__);}

bool APlayerCharacterBase::DEBUG_ServerSpawnReplay_Validate() {DebugPrint(__FILE__, __LINE__);
	return true;
DebugPrint(__FILE__, __LINE__);}

void APlayerCharacterBase::DEBUG_ServerSpawnReplay_Implementation() {DebugPrint(__FILE__, __LINE__);
	FActorSpawnParameters spawnParams;
	spawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

	// Spawn ReplayCharacter
	AReplayCharacterBase* newPlayer = GetWorld()->SpawnActor<AReplayCharacterBase>(ReplayCharacter, DEBUG_position, DEBUG_rotation, spawnParams);
	if(!AssertNotNull(newPlayer, GetWorld(), __FILE__, __LINE__, "Could not spawn replay character!")) {DebugPrint(__FILE__, __LINE__);
		return;
	DebugPrint(__FILE__, __LINE__);}

	// Start Replay on spawned ReplayCharacter
	PlayerStateRecorder = FindComponentByClass<UPlayerStateRecorder>();
	AssertNotNull(PlayerStateRecorder, GetWorld(), __FILE__, __LINE__);
	newPlayer->SetReplayData(PlayerStateRecorder->RecordedPlayerStates);
	Cast<AAIControllerBase>(newPlayer->GetController())->Initialize(Cast<APlayerController>(GetController()));

	newPlayer->StartReplay();
DebugPrint(__FILE__, __LINE__);}

void APlayerCharacterBase::DEBUG_StartRecording() {DebugPrint(__FILE__, __LINE__);
	DEBUG_ServerSavePosition();
	PlayerStateRecorder->ServerStartRecording();
DebugPrint(__FILE__, __LINE__);}

bool APlayerCharacterBase::DEBUG_ServerSavePosition_Validate() {DebugPrint(__FILE__, __LINE__);
	return true;
DebugPrint(__FILE__, __LINE__);}

void APlayerCharacterBase::DEBUG_ServerSavePosition_Implementation() {DebugPrint(__FILE__, __LINE__);
	DEBUG_position = GetActorLocation();
	DEBUG_rotation = GetActorRotation();
DebugPrint(__FILE__, __LINE__);}

void APlayerCharacterBase::ClientStartRecording_Implementation() {DebugPrint(__FILE__, __LINE__);
	PlayerStateRecorder->ServerStartRecording();
DebugPrint(__FILE__, __LINE__);}

void APlayerCharacterBase::ServerStartFire_Implementation() {DebugPrint(__FILE__, __LINE__);
	UPlayerStateRecorder* recorder = FindComponentByClass<UPlayerStateRecorder>();
	AssertNotNull(recorder, GetWorld(), __FILE__, __LINE__);

	recorder->ServerRecordKeyPressed("Fire");

	Super::ServerStartFire_Implementation();
DebugPrint(__FILE__, __LINE__);}

void APlayerCharacterBase::ServerStopFire_Implementation() {DebugPrint(__FILE__, __LINE__);
	UPlayerStateRecorder* recorder = FindComponentByClass<UPlayerStateRecorder>();
	AssertNotNull(recorder, GetWorld(), __FILE__, __LINE__);

	recorder->ServerRecordKeyReleased("Fire");
	Super::ServerStopFire_Implementation();
DebugPrint(__FILE__, __LINE__);}