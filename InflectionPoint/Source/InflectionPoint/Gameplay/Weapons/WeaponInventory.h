// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "BaseWeapon.h"
#include "WeaponInventory.generated.h"


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
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	virtual bool IsReadyForInitialization();

	UPROPERTY(EditDefaultsOnly, Category = Weapons)
		TArray<TSubclassOf<ABaseWeapon>> DefaultWeaponClasses;

	UPROPERTY(EditDefaultsOnly, Category = Weapons)
		TArray<TSubclassOf<ABaseWeapon>> DisabledWeapons;

	void Destroy();

	/** Returns NULL if the weapon is disabled */
	UFUNCTION(BlueprintCallable)
		ABaseWeapon* GetWeapon(int index);
	
	ABaseWeapon* GetRandomWeapon();
	
	UFUNCTION(BlueprintCallable)
		int GetWeaponNum();
	
	UFUNCTION(BlueprintCallable)
		ABaseWeapon* GetWeaponByClass(UClass* weaponClass);

	ABaseWeapon* GetNextWeapon(ABaseWeapon* CurrentWeapon);

	ABaseWeapon* GetPreviousWeapon(ABaseWeapon* CurrentWeapon);

	bool IsWeaponDisabled(UClass* weaponClass);

	void SetWeaponDisableStatus(UClass* weaponClass, bool disabled);
private:
	TArray<ABaseWeapon*> weapons;

};
