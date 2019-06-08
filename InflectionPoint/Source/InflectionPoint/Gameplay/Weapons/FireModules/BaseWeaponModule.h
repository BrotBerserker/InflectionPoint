// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "BaseWeaponModule.generated.h"

/**
 *
 */
UCLASS(hidecategories = Object, BlueprintType)
class INFLECTIONPOINT_API UBaseWeaponModule : public UObject, public FTickableGameObject {
	GENERATED_BODY()
public:
	/* ---------------------- */
	/*    Editor Settings     */
	/* ---------------------- */

	/** Sound to play when no Ammo is left */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Sound)
		class USoundBase* NoAmmoSound;

	/** Sound to play each time we fire */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Sound)
		class USoundBase* ChargeSound;

	/** Sound to play each time we fire */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Sound)
		class USoundBase* FireSound;

	/** Sound to play each while fireing */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Sound)
		class USoundBase* FireLoopSound;
	/** FX for muzzle flash */
	UPROPERTY(EditDefaultsOnly, Category = Effects)
		UParticleSystem* MuzzleFX;

	/** Can be used to scale the muzzle fx */
	UPROPERTY(EditDefaultsOnly, Category = Effects)
		FVector MuzzleFXScale = FVector(1.f, 1.f, 1.f);

	/** Duration for the muzzle FX flash (-1 for endless)*/
	UPROPERTY(EditDefaultsOnly, Category = Effects)
		float MuzzleFXDuration = 0.1;

	/** CameraShake when fiering */
	UPROPERTY(EditDefaultsOnly, Category = Effects)
		TSubclassOf<UCameraShake> FireCameraShake;
	/** Whether automatic fire should be enabled for this weapon */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = WeaponConfig)
		bool AutoFire = true;

	/** Seconds to wait for the first shot */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = WeaponConfig)
		float ChargeDuration = 0.0f;

	/** Seconds to wait between two shots */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = WeaponConfig)
		float FireInterval = 1.0f;

	/** Delay before reloading */
	UPROPERTY(EditDefaultsOnly, Category = WeaponConfig)
		float ReloadDelay = 0;

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
	virtual void Tick(float DeltaTime) override {};
	virtual bool IsTickable() const override { return false; }
	virtual TStatId GetStatId() const override;

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
	virtual void OnDeactivate();
};
