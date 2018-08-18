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

	void Destroy();

	void AddWeapon(ABaseWeapon* Weapon);

	void RemoveWeapon(ABaseWeapon* Weapon);

	UFUNCTION(BlueprintCallable)
		ABaseWeapon* GetWeapon(int index);
	
	UFUNCTION(BlueprintCallable)
		int GetWeaponNum();

	ABaseWeapon* GetNextWeapon(ABaseWeapon* CurrentWeapon);

	ABaseWeapon* GetPreviousWeapon(ABaseWeapon* CurrentWeapon);

private:
	TArray<ABaseWeapon*> weapons;

};
