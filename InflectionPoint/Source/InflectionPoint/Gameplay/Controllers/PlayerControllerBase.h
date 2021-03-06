// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/PlayerController.h"
#include "Gameplay/Recording/PlayerStateRecorder.h"
#include "Gameplay/CharacterInfoProvider.h"
#include "Gameplay/Shop/BaseShopItem.h"
#include "Gameplay/Weapons/WeaponInventory.h"
#include "Gamemodes/TDMPlayerStateBase.h"
#include "Gameplay/Weapons/BaseWeapon.h"
#include "PlayerControllerBase.generated.h"

USTRUCT(BlueprintType)
struct FRecordedCharacterData {
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadWrite)
		TSubclassOf<ABaseCharacter> CharacterClass;

	UPROPERTY(BlueprintReadWrite)
		int Phase;

	UPROPERTY(BlueprintReadWrite)
		float TimeStamp;

	UPROPERTY(BlueprintReadWrite)
		TArray<FRecordedPlayerState> RecordedPlayerStates;
};

USTRUCT(BlueprintType)
struct FRecordedPlayerData {
	GENERATED_BODY()

public:
	int Phase;
	TArray<FRecordedPlayerState> RecordedPlayerStates;
	TArray<FTDMEquipSlot> EquippedShopItems;
};

UCLASS()
class INFLECTIONPOINT_API APlayerControllerBase : public APlayerController {
	GENERATED_BODY()
public:
	APlayerControllerBase(const FObjectInitializer& ObjectInitializer);

	void BeginPlay() override;

	void Tick(float DeltaTime) override;

	void UpdateCharactersInLineOfSight();

	void OnPossess(APawn* InPawn) override;

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
		void ClientShowMatchEnd(int winningTeam);

	UFUNCTION(Client, Reliable)
		void ClientShowRoundEnd(int winningTeam);

	UFUNCTION(BlueprintImplementableEvent)
		void OnShowMatchEnd(int winningTeam);

	UFUNCTION(BlueprintImplementableEvent)
		void OnShowRoundEnd(int winningTeam);

	UFUNCTION(Client, Reliable)
		void ClientShowPhaseCountdownNumber(int number);

	UFUNCTION(BlueprintImplementableEvent)
		void OnPhaseCountdownUpdate(int number);

	UFUNCTION(Client, Reliable)
		void ClientShowMatchCountdownNumber(int number);

	UFUNCTION(BlueprintImplementableEvent)
		void OnMatchCountdownUpdate(int number);

	UFUNCTION(Client, Reliable)
		void ClientShowShopCountdownNumber(int number);

	UFUNCTION(BlueprintImplementableEvent)
		void OnShopCountdownUpdate(int number);

	UFUNCTION(CLient, Reliable)
		void ClientShowShop(bool intelligentSmartResetBoolean);

	UFUNCTION(BlueprintImplementableEvent)
		void OnShowShop(bool intelligentSmartResetBoolean);

	UFUNCTION(Client, Reliable)
		void ClientSetIgnoreInput(bool ignore);

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
		void DamageDealt(bool headshot);

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

	UFUNCTION()
		bool IsLookingAtActor(AActor* actor, float distance = 130);

	UFUNCTION(Server, Reliable, BlueprintCallable, WithValidation)
		void ServerPurchaseShopItem(TSubclassOf<class UBaseShopItem> itemClass);

	UFUNCTION(Server, Reliable, BlueprintCallable, WithValidation)
		void ServerEquipShopItem(EInventorySlotPosition inventorySlot, TSubclassOf<class UBaseShopItem> item);

	UFUNCTION(Server, Reliable, BlueprintCallable, WithValidation)
		void ServerUnequipShopItemFromSlot(EInventorySlotPosition slot);

	UFUNCTION(BlueprintCallable)
		TArray<FRecordedPlayerState> GetRecordedCharacterData(TSubclassOf<ABaseCharacter> CharacterClass, int Phase, float TimeStamp);

public:
	UPROPERTY(BlueprintReadWrite, Replicated)
		class ABaseCharacter* SpectatedCharacter;

	UPROPERTY(BlueprintReadWrite)
		TArray<ABaseCharacter*> CharactersInLineOfSight;

	UPROPERTY(BlueprintReadWrite)
		TArray<FRecordedPlayerData> RecordedPlayerData;

	UPROPERTY(BlueprintReadWrite)
		TArray<FRecordedCharacterData> RecordedCharacterData;
private:
	/** Searches the given array for an actor that can be spectated. If one is found, switches the camera to spectate him and returns true */
	bool SpectateNextActorInRange(TArray<AActor*> actors, int32 beginIndex, int32 endIndex);

};

