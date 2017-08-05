// Fill out your copyright notice in the Description page of Project Settings.

#include "InflectionPoint.h"
#include "PlayerControlledFPSCharacter.h"
#include "Gameplay/Characters/ReplayControlledFPSCharacter.h"
#include "Utils/CheckFunctions.h"


//////////////////////////////////////////////////////////////////////////
// Input

void APlayerControlledFPSCharacter::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) {
	// set up gameplay key bindings
	check(PlayerInputComponent);

	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ACharacter::Jump);
	PlayerInputComponent->BindAction("Jump", IE_Released, this, &ACharacter::StopJumping);

	PlayerInputComponent->BindAction("Fire", IE_Pressed, this, &ABaseCharacter::OnFire);
	PlayerInputComponent->BindAction("DEBUG_Fire", IE_Pressed, this, &ABaseCharacter::OnDebugFire);

	PlayerInputComponent->BindAxis("MoveForward", this, &ABaseCharacter::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &ABaseCharacter::MoveRight);

	PlayerInputComponent->BindAxis("Turn", this, &APlayerControlledFPSCharacter::TurnAtRate);
	PlayerInputComponent->BindAxis("LookUp", this, &APlayerControlledFPSCharacter::LookUpAtRate);

	// DEBUG Bindings
	PlayerInputComponent->BindAction("DEBUG_SpawnReplay", IE_Pressed, this, &APlayerControlledFPSCharacter::DEBUG_ServerSpawnReplay);
	PlayerInputComponent->BindAction("DEBUG_StartRecording", IE_Pressed, this, &APlayerControlledFPSCharacter::DEBUG_StartRecording);

	// Initialize and start PlayerStateRecorder
	PlayerStateRecorder = FindComponentByClass<UPlayerStateRecorder>();
	AssertNotNull(PlayerStateRecorder, GetWorld(), __FILE__, __LINE__);
	PlayerStateRecorder->InitializeBindings(PlayerInputComponent);

	// Controller bindings
	//PlayerInputComponent->BindAxis("TurnRate", this, &ABaseCharacter::TurnAtRate);
	//PlayerInputComponent->BindAxis("LookUpRate", this, &ABaseCharacter::LookUpAtRate);
}

bool APlayerControlledFPSCharacter::DEBUG_ServerSpawnReplay_Validate() {
	return true;
}

void APlayerControlledFPSCharacter::DEBUG_ServerSpawnReplay_Implementation() {
	FActorSpawnParameters spawnParams;
	spawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

	// Spawn ReplayCharacter
	AReplayControlledFPSCharacter* newPlayer = GetWorld()->SpawnActor<AReplayControlledFPSCharacter>(ReplayCharacter, DEBUG_position, DEBUG_rotation, spawnParams);
	if(!AssertNotNull(newPlayer, GetWorld(), __FILE__, __LINE__, "Could not spawn replay character!")) {
		return;
	}

	// Start Replay on spawned ReplayCharacter
	PlayerStateRecorder = FindComponentByClass<UPlayerStateRecorder>();
	AssertNotNull(PlayerStateRecorder, GetWorld(), __FILE__, __LINE__);
	newPlayer->SetReplayData(PlayerStateRecorder->RecordedPlayerStates);
	newPlayer->OwningPlayerController = Cast<APlayerController>(GetController());
	newPlayer->StartReplay();
}

void APlayerControlledFPSCharacter::DEBUG_StartRecording() {
	DEBUG_ServerSavePosition();
	PlayerStateRecorder->ServerStartRecording();
}

bool APlayerControlledFPSCharacter::DEBUG_ServerSavePosition_Validate() {
	return true;
}

void APlayerControlledFPSCharacter::DEBUG_ServerSavePosition_Implementation() {
	DEBUG_position = GetActorLocation();
	DEBUG_rotation = GetActorRotation();
}

void APlayerControlledFPSCharacter::ClientStartRecording_Implementation() {
	PlayerStateRecorder->ServerStartRecording();
}

void APlayerControlledFPSCharacter::ClientSetIgnoreInput_Implementation(bool ignore) {
	APlayerController* controller = (APlayerController*)GetController();
	AssertNotNull(controller, GetWorld(), __FILE__, __LINE__);
	if(ignore) {
		DisableInput(controller);
	} else {
		EnableInput(controller);
	}
}

void APlayerControlledFPSCharacter::ClientShowCountdownNumber_Implementation(int number) {
	OnCountdownUpdate(number);
}