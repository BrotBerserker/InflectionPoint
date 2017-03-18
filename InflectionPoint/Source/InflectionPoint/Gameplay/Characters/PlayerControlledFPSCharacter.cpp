// Fill out your copyright notice in the Description page of Project Settings.

#include "InflectionPoint.h"
#include "PlayerControlledFPSCharacter.h"
#include "Gameplay/Recording/InputRecorder.h"
#include "Gameplay/Characters/ReplayControlledFPSCharacter.h"
#include "Gameplay/Recording/PositionRecorder.h"
#include "Gameplay/Replaying/PositionCorrector.h"

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


	// We have 2 versions of the rotation bindings to handle different kinds of devices differently
	// "turn" handles devices that provide an absolute delta, such as a mouse.
	// "turnrate" is for devices that we choose to treat as a rate of change, such as an analog joystick
	PlayerInputComponent->BindAxis("Turn", this, &ABaseCharacter::TurnAtRate);
	//PlayerInputComponent->BindAxis("Turn", this, &APawn::AddControllerYawInput);
	//PlayerInputComponent->BindAxis("TurnRate", this, &ABaseCharacter::TurnAtRate);
	PlayerInputComponent->BindAxis("LookUp", this, &ABaseCharacter::LookUpAtRate);
	//PlayerInputComponent->BindAxis("LookUp", this, &APawn::AddControllerPitchInput);
	//PlayerInputComponent->BindAxis("LookUpRate", this, &ABaseCharacter::LookUpAtRate);

	// DEBUG Bindings
	PlayerInputComponent->BindAction("DEBUG_SpawnReplay", IE_Pressed, this, &APlayerControlledFPSCharacter::DEBUG_SpawnReplay);
}

void APlayerControlledFPSCharacter::DEBUG_SpawnReplay() {

	AActor* playerStart = GetWorld()->GetAuthGameMode()->FindPlayerStart(GetWorld()->GetFirstPlayerController());

	FVector loc = playerStart->GetTransform().GetLocation();
	FRotator rot = FRotator(playerStart->GetTransform().GetRotation());

	AReplayControlledFPSCharacter* newPlayer = GetWorld()->SpawnActor<AReplayControlledFPSCharacter>(ReplayCharacter, loc, rot);
	if(newPlayer == nullptr) {
		return;
	}

	// Replay inputs
	UInputRecorder* inputRecorder = FindComponentByClass<UInputRecorder>();
	newPlayer->StartReplay(inputRecorder->Inputs, inputRecorder->Yaws, inputRecorder->Pitches, inputRecorder->MovementsForward, inputRecorder->MovementsRight);

	// Correct positions
	UPositionRecorder* posRecorder = FindComponentByClass<UPositionRecorder>();
	newPlayer->FindComponentByClass<UPositionCorrector>()->StartCorrecting(posRecorder->RecordArray);
}

