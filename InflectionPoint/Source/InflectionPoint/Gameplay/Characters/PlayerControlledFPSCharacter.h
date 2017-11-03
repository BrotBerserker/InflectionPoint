// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "BaseCharacter.h"
#include "Gameplay/Recording/PlayerStateRecorder.h"
#include "PlayerControlledFPSCharacter.generated.h"


/**
 *
 */
UCLASS()
class INFLECTIONPOINT_API APlayerControlledFPSCharacter : public ABaseCharacter {
	GENERATED_BODY()

public:

	/** Override from ACharacter */
	virtual void SetupPlayerInputComponent(UInputComponent* InputComponent) override;

	/** Only for Debug purposes, should be configured inside the GameMode */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Debug)
		class UClass* ReplayCharacter;

	/** Deletes current recordings and starts new recording for debug purposes */
	UFUNCTION()
		void DEBUG_StartRecording();

	/** Saves the current location and rotation for debug purposes */
	UFUNCTION(Reliable, Server, WithValidation)
		void DEBUG_ServerSavePosition();

	/** Spawns a ReplayCharacter for debug purposes */
	UFUNCTION(Reliable, Server, WithValidation)
		void DEBUG_ServerSpawnReplay();

	FVector DEBUG_position;

	FRotator DEBUG_rotation;

	UPlayerStateRecorder* PlayerStateRecorder;

	UFUNCTION(Client, Reliable)
		void ClientStartRecording();

	UFUNCTION(Client, Reliable)
		void ClientSetIgnoreInput(bool ignore);

	UFUNCTION(Client, Reliable)
		void ClientShowCountdownNumber(int number);

	UFUNCTION(BlueprintImplementableEvent)
		void OnCountdownUpdate(int number);

	UFUNCTION(Client, Unreliable)
		void ClientShowKillInfo(const FString& Killer, const FString& Killed, UTexture2D* WeaponImage);

	UFUNCTION(BlueprintImplementableEvent)
		void OnKillInfoAdded(const FString& Killer, const FString& Killed, UTexture2D* WeaponImage);

	void FireProjectile(TSubclassOf<AInflectionPointProjectile> &projectileClassToSpawn) override;

	void StopFire() override;
};
