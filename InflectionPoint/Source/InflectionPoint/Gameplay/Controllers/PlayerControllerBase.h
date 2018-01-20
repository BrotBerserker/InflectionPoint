// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/PlayerController.h"
#include "Gameplay/CharacterInfoProvider.h"
#include "PlayerControllerBase.generated.h"

/**
 *
 */
UCLASS()
class INFLECTIONPOINT_API APlayerControllerBase : public APlayerController {
	GENERATED_BODY()
public:
	APlayerControllerBase(const FObjectInitializer& ObjectInitializer);

	void BeginPlay() override;

	void Possess(APawn* InPawn) override;

	UFUNCTION(Client, Reliable)
		void ClientSetControlRotation(FRotator rotation);

	UPROPERTY()
		class UCharacterInfoProvider* CharacterInfoProvider;

	UFUNCTION(Client, Reliable)
		void ClientRoundStarted(int Round);

	UFUNCTION(BlueprintImplementableEvent)
		void OnRoundStarted(int Round);

	UFUNCTION(Client, Unreliable)
		void ClientShowKillInfo(FCharacterInfo KilledInfo, float killedScoreChange, FCharacterInfo KillerInfo, float killerScoreChange, UTexture2D* WeaponImage);

	UFUNCTION(BlueprintImplementableEvent)
		void OnKillInfoAdded(FCharacterInfo KilledInfo, float killedScoreChange, FCharacterInfo KillerInfo, float killerScoreChange, UTexture2D* WeaponImage);

	UFUNCTION(Client, Reliable)
		void ClientShowCountdownNumber(int number);

	UFUNCTION(BlueprintImplementableEvent)
		void OnCountdownUpdate(int number);

	UFUNCTION(Client, Reliable)
		void ClientSetIgnoreInput(bool ignore);

	/** Spectates the next player or replay that is alive and in the same team as this player */
	UFUNCTION(BlueprintCallable, Server, WithValidation, Unreliable)
		void ServerSwitchSpectatedPlayer();	

	/** Spectates the next player or replay that is alive and in the same team as this player */
	UFUNCTION(BlueprintImplementableEvent)
		void SpactatingCharacterSwitched(ABaseCharacter* newCharacter, FCharacterInfo newCharacterInfo);
	

private:
	/** Searches the given array for an actor that can be spectated. If one is found, switches the camera to spectate him and returns true */
	bool SpectateNextActorInRange(TArray<AActor*> actors, int32 beginIndex, int32 endIndex);

};

