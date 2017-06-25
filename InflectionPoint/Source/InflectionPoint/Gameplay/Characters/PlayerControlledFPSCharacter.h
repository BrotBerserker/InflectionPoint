// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "BaseCharacter.h"
#include "Gameplay/Recording/InputRecorder.h"
#include "Gameplay/Recording/PlayerStateRecorder.h"
#include "PlayerControlledFPSCharacter.generated.h"


/**
 *
 */
UCLASS()
class INFLECTIONPOINT_API APlayerControlledFPSCharacter : public ABaseCharacter {
	GENERATED_BODY()

protected:

	/** Override from ACharacter */
	virtual void SetupPlayerInputComponent(UInputComponent* InputComponent) override;

	/** Only for Debug purposes, should be configured inside the GameMode */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Debug)
		class UClass* ReplayCharacter;

	/** Spawns a ReplayCharacter for Debug purposes, the real spawning should happen in the GameMode */
	UFUNCTION(Reliable, Server, WithValidation)
		void DEBUG_ServerSpawnReplay();

	UPlayerStateRecorder* PlayerStateRecorder;
};
