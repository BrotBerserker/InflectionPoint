// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/PlayerController.h"
#include "Gameplay/CharacterInfoProvider.h"
#include "Gameplay/Weapons/BaseWeapon.h"
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

	UFUNCTION(Client, Reliable)
		void ClientPhaseStarted(int Phase);

	UFUNCTION(BlueprintImplementableEvent)
		void OnPhaseStarted(int Phase);

	UFUNCTION(Client, Reliable)
		void ClientShowKillInfo(FCharacterInfo KilledInfo, float killedScoreChange, FCharacterInfo KillerInfo, float killerScoreChange, UTexture2D* WeaponImage);

	UFUNCTION(BlueprintImplementableEvent)
		void OnKillInfoAdded(FCharacterInfo KilledInfo, float killedScoreChange, FCharacterInfo KillerInfo, float killerScoreChange, UTexture2D* WeaponImage);

	UFUNCTION(Client, Reliable)
		void ClientShowCountdownNumber(int number);

	UFUNCTION(BlueprintImplementableEvent)
		void OnCountdownUpdate(int number);

	UFUNCTION(Client, Reliable)
		void ClientSetIgnoreInput(bool ignore);

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
		void DamageDealt();

	/** Spectates the next player or replay that is alive and in the same team as this player */
	UFUNCTION(BlueprintCallable, Server, WithValidation, Unreliable)
		void ServerSwitchSpectatedCharacter();	

	/** Spectates the next player or replay that is alive and in the same team as this player */
	UFUNCTION(BlueprintImplementableEvent)
		void SpectatedCharacterSwitched(ABaseCharacter* newCharacter, FCharacterInfo newCharacterInfo);
	
	UFUNCTION(BlueprintImplementableEvent)
		void OnStartAiming(ABaseWeapon* weapon);

	UFUNCTION(BlueprintImplementableEvent)
		void OnStopAiming(ABaseWeapon* weapon);

	UFUNCTION(BlueprintImplementableEvent)
		void OnWeaponChanged(ABaseWeapon* newWeapon, ABaseWeapon* oldWeapon);
public:
	UPROPERTY(BlueprintReadWrite, Replicated)
		class ABaseCharacter* SpectatedCharacter;

private:
	/** Searches the given array for an actor that can be spectated. If one is found, switches the camera to spectate him and returns true */
	bool SpectateNextActorInRange(TArray<AActor*> actors, int32 beginIndex, int32 endIndex);

};

