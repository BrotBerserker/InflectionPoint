// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "BaseWeaponModule.generated.h"

/**
 *
 */
UCLASS(hidecategories = Object, customconstructor, BlueprintType, config = Engine)
class INFLECTIONPOINT_API UBaseWeaponModule : public UObject {
	GENERATED_BODY()
public:
	/* ---------------------- */
	/*    Editor Settings     */
	/* ---------------------- */

	/** How many shots are fired at once */
	UPROPERTY(EditDefaultsOnly, Category = WeaponConfig)
		int FireShotNum = 1;

public:
	UPROPERTY(BlueprintReadOnly)
		class ABaseWeapon* Weapon;
	UPROPERTY(BlueprintReadOnly)
		class ABaseCharacter* OwningCharacter;

public:
	/* ------------- */
	/*   Functions   */
	/* ------------- */

	/** Fires a shot (includes animation, sound, and decreasing ammo) */
	virtual void Fire();

	/** Returns true if firing should be possible */
	virtual bool CanFire();

	/** Called before ExecuteFire */
	virtual void PreExecuteFire();

	/** This function should be overriden in subclasses to implement specific fire behaviour */
	virtual void ExecuteFire() PURE_VIRTUAL(ABaseWeapon::ExecuteFire, ;);

	/** Called after ExecuteFire*/
	virtual void PostExecuteFire();

	/** Called when this module is attached to the weapon */
	virtual void OnActivate();

	/** Called when this module is detached from the weapon */
	virtual void OnDetactivate();
};
