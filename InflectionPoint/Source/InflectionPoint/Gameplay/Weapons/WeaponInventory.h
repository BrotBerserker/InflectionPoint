// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "BaseWeapon.h"
#include "WeaponInventory.generated.h"

UENUM(Blueprintable)
enum class EInventorySlot : uint8 {
	Weapon1 = 0,
	Weapon2,
	Weapon3,
	// ======
	Skill1,
	Skill2,
	Skill3,
};

USTRUCT(BlueprintType)
struct FInventoryWeaponSlot {
	GENERATED_BODY()

		UPROPERTY(EditAnywhere, BlueprintReadonly)
		EInventorySlot SlotType;

	UPROPERTY(EditAnywhere, BlueprintReadonly)
		TSubclassOf<ABaseWeapon> DefaultWeapon;

	UPROPERTY(BlueprintReadonly)
		ABaseWeapon* Weapon;
};

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class INFLECTIONPOINT_API UWeaponInventory : public UActorComponent {
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	UWeaponInventory();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:

	virtual bool IsReadyForInitialization();

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Replicated, Category = Weapons)
		TArray<FInventoryWeaponSlot> WeaponSlots;

	void Destroy();

	UFUNCTION(BlueprintCallable)
		void SetWeaponAtPosition(EInventorySlot slot, TSubclassOf<ABaseWeapon> weaponClass);

	UFUNCTION(BlueprintCallable)
		ABaseWeapon* GetWeapon(EInventorySlot slot);

	UFUNCTION(BlueprintCallable)
		int GetWeaponNum();

	UFUNCTION(BlueprintCallable)
		ABaseWeapon* GetWeaponByClass(UClass* weaponClass);

	ABaseWeapon* GetRandomWeapon();

	ABaseWeapon* GetNextWeapon(ABaseWeapon* CurrentWeapon);

	ABaseWeapon* GetNextUsableWeapon(ABaseWeapon* CurrentWeapon);

	ABaseWeapon* GetPreviousWeapon(ABaseWeapon* CurrentWeapon);

	ABaseWeapon* GetPreviousUsableWeapon(ABaseWeapon* CurrentWeapon);
private:

	ABaseWeapon* GetNextWeaponInDirection(ABaseWeapon* CurrentWeapon, bool isDirectionForward);
	int GetWeaponSlotIndex(ABaseWeapon* weapon);
	int GetWeaponSlotIndex(EInventorySlot type);

	void ClearWeaponSlot(FInventoryWeaponSlot slot);

	ABaseWeapon* SpawnWeapon(TSubclassOf<ABaseWeapon> weapon);

	void InitDefaultWeapons();
};
