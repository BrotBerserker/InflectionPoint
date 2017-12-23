// Fill out your copyright notice in the Description page of Project Settings.

#include "InflectionPoint.h"
#include "PlayerCharacterBase.h"
#include "Gameplay/Characters/ReplayCharacterBase.h"
#include "Gameplay/Weapons/InflectionPointProjectile.h"
#include "Gameplay/Controllers/AIControllerBase.h"
#include "Utils/CheckFunctions.h"

bool APlayerCharacterBase::IsReadyForInitialization() {
	if(!PlayerState) {
		return false;
	}
	return true;
}

void APlayerCharacterBase::Initialize() {
	ApplyPlayerColor(Cast<ATDMPlayerStateBase>(PlayerState));
	ShowSpawnAnimation();
}


//////////////////////////////////////////////////////////////////////////
// Input

void APlayerCharacterBase::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) {
	// set up gameplay key bindings
	check(PlayerInputComponent);

	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ACharacter::Jump);
	PlayerInputComponent->BindAction("Jump", IE_Released, this, &ACharacter::StopJumping);

	PlayerInputComponent->BindAction("Fire", IE_Pressed, this, &ABaseCharacter::OnFire);
	PlayerInputComponent->BindAction("DEBUG_Fire", IE_Pressed, this, &ABaseCharacter::OnDebugFire);

	PlayerInputComponent->BindAction("Fire", IE_Released, this, &ABaseCharacter::OnStopFire);
	PlayerInputComponent->BindAction("DEBUG_Fire", IE_Released, this, &ABaseCharacter::OnStopFire);

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

	// Controller bindings
	//PlayerInputComponent->BindAxis("TurnRate", this, &ABaseCharacter::TurnAtRate);
	//PlayerInputComponent->BindAxis("LookUpRate", this, &ABaseCharacter::LookUpAtRate);
}

bool APlayerCharacterBase::DEBUG_ServerSpawnReplay_Validate() {
	return true;
}

void APlayerCharacterBase::DEBUG_ServerSpawnReplay_Implementation() {
	FActorSpawnParameters spawnParams;
	spawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

	// Spawn ReplayCharacter
	AReplayCharacterBase* newPlayer = GetWorld()->SpawnActor<AReplayCharacterBase>(ReplayCharacter, DEBUG_position, DEBUG_rotation, spawnParams);
	if(!AssertNotNull(newPlayer, GetWorld(), __FILE__, __LINE__, "Could not spawn replay character!")) {
		return;
	}

	// Start Replay on spawned ReplayCharacter
	PlayerStateRecorder = FindComponentByClass<UPlayerStateRecorder>();
	AssertNotNull(PlayerStateRecorder, GetWorld(), __FILE__, __LINE__);
	newPlayer->SetReplayData(PlayerStateRecorder->RecordedPlayerStates);
	Cast<AAIControllerBase>(newPlayer->GetController())->Initialize(Cast<APlayerController>(GetController()));
	
	newPlayer->StartReplay();
}

void APlayerCharacterBase::DEBUG_StartRecording() {
	DEBUG_ServerSavePosition();
	PlayerStateRecorder->ServerStartRecording();
}

bool APlayerCharacterBase::DEBUG_ServerSavePosition_Validate() {
	return true;
}

void APlayerCharacterBase::DEBUG_ServerSavePosition_Implementation() {
	DEBUG_position = GetActorLocation();
	DEBUG_rotation = GetActorRotation();
}

void APlayerCharacterBase::ClientStartRecording_Implementation() {
	PlayerStateRecorder->ServerStartRecording();
}

void APlayerCharacterBase::FireProjectile(TSubclassOf<AInflectionPointProjectile> &projectileClassToSpawn) {
	UPlayerStateRecorder* recorder = FindComponentByClass<UPlayerStateRecorder>();
	AssertNotNull(recorder, GetWorld(), __FILE__, __LINE__);

	if(projectileClassToSpawn == ProjectileClass) {
		recorder->RecordKeyPressed("Fire");
	} else if(projectileClassToSpawn == DebugProjectileClass) {
		recorder->RecordKeyPressed("DEBUG_Fire");
	}
	Super::FireProjectile(projectileClassToSpawn);
}

void APlayerCharacterBase::StopFire() {
	UPlayerStateRecorder* recorder = FindComponentByClass<UPlayerStateRecorder>();
	AssertNotNull(recorder, GetWorld(), __FILE__, __LINE__);

	recorder->RecordKeyReleased("Fire");
	recorder->RecordKeyReleased("DEBUG_Fire");
	Super::StopFire();
}