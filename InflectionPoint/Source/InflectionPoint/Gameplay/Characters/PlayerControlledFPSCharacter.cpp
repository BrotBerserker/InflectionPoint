// Fill out your copyright notice in the Description page of Project Settings.

#include "InflectionPoint.h"
#include "PlayerControlledFPSCharacter.h"
#include "Gameplay/Characters/ReplayControlledFPSCharacter.h"
#include "Gameplay/Recording/PositionRecorder.h"
#include "Gameplay/Recording/RotationRecorder.h"
#include "Gameplay/Replaying/PositionCorrector.h"
#include "Gameplay/Replaying/RotationReplayer.h"
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

	PlayerInputComponent->BindAxis("Turn", this, &APawn::AddControllerYawInput);
	PlayerInputComponent->BindAxis("LookUp", this, &APawn::AddControllerPitchInput);

	// DEBUG Bindings
	PlayerInputComponent->BindAction("DEBUG_SpawnReplay", IE_Pressed, this, &APlayerControlledFPSCharacter::DEBUG_SpawnReplay);

	InputRecorder = FindComponentByClass<UInputRecorder>();
	AssertNotNull(InputRecorder, GetWorld(), __FILE__, __LINE__);
	InputRecorder->InitializeBindings(PlayerInputComponent);

	// Controller bindings
	//PlayerInputComponent->BindAxis("TurnRate", this, &ABaseCharacter::TurnAtRate);
	//PlayerInputComponent->BindAxis("LookUpRate", this, &ABaseCharacter::LookUpAtRate);
}

void APlayerControlledFPSCharacter::DEBUG_SpawnReplay() {

	AActor* playerStart = GetWorld()->GetAuthGameMode()->FindPlayerStart(GetWorld()->GetFirstPlayerController());
	AssertNotNull(playerStart, GetWorld(), __FILE__, __LINE__);

	FVector loc = playerStart->GetTransform().GetLocation();
	FRotator rot = FRotator(playerStart->GetTransform().GetRotation());

	AReplayControlledFPSCharacter* newPlayer = GetWorld()->SpawnActor<AReplayControlledFPSCharacter>(ReplayCharacter, loc, rot);
	if(!AssertNotNull(newPlayer, GetWorld(), __FILE__, __LINE__, "Could not spawn replay character!")) {
		return;
	}

	// Replay inputs
	AssertNotNull(InputRecorder, GetWorld(), __FILE__, __LINE__);
	newPlayer->StartReplay(InputRecorder->Inputs, InputRecorder->MovementsForward, InputRecorder->MovementsRight);

	// Correct positions
	UPositionRecorder* posRecorder = FindComponentByClass<UPositionRecorder>();
	AssertNotNull(posRecorder, GetWorld(), __FILE__, __LINE__);
	newPlayer->FindComponentByClass<UPositionCorrector>()->StartCorrecting(posRecorder->RecordArray);

	// Replay rotations
	URotationRecorder* rotRecorder = FindComponentByClass<URotationRecorder>();
	AssertNotNull(rotRecorder, GetWorld(), __FILE__, __LINE__);
	newPlayer->FindComponentByClass<URotationReplayer>()->StartReplay(rotRecorder->Yaws, rotRecorder->Pitches);
}
