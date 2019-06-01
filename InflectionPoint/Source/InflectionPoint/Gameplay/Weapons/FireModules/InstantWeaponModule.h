// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Gameplay/Weapons/FireModules/BaseWeaponModule.h"
#include "Gameplay/Characters/BaseCharacter.h"
#include "Gameplay/Weapons/BaseWeapon.h"
#include "DebugTools/DebugLineDrawer.h"
#include "InstantWeaponModule.generated.h"

/**
 *
 */
UCLASS()
class INFLECTIONPOINT_API UInstantWeaponModule : public UBaseWeaponModule {
	GENERATED_BODY()
public:
	UInstantWeaponModule();

public:
	/* ---------------------- */
	/*    Editor Settings     */
	/* ---------------------- */

	/** base weapon spread (degrees) */
	UPROPERTY(EditDefaultsOnly, Category = WeaponConfig)
		float Spread = 0;

	/** spread to use when the player is aiming */
	UPROPERTY(EditDefaultsOnly, Category = WeaponConfig)
		float AimSpread = 0;

	/** weapon range */
	UPROPERTY(EditDefaultsOnly, Category = WeaponConfig)
		float Range = 10000;

	/** weapon shoot radius */
	UPROPERTY(EditDefaultsOnly, Category = WeaponConfig)
		float ShootRadius = 0;

	/** damage amount */
	UPROPERTY(EditDefaultsOnly, Category = WeaponConfig)
		int32 Damage = 10;

	/** damage amount that gets added if hit is a headshot */
	UPROPERTY(EditDefaultsOnly, Category = WeaponConfig)
		int32 HeadshotBonusDamage = 0;

	/** type of damage */
	UPROPERTY(EditDefaultsOnly, Category = WeaponConfig)
		TSubclassOf<UDamageType> DamageType;

	/** Collision Channel for the raycast*/
	UPROPERTY(EditAnywhere, meta = (Bitmask, BitmaskEnum = "ECollisionChannel"))
		int32 CollisionChannel;

	/** smoke trail */
	UPROPERTY(EditDefaultsOnly, Category = Effects)
		UParticleSystem* TrailFX;

	/** param name for the trail's source */
	UPROPERTY(EditDefaultsOnly, Category = Effects)
		FName TrailSourceParamName;

	/** param name for the trail's target */
	UPROPERTY(EditDefaultsOnly, Category = Effects)
		FName TrailTargetParamName;

	/** FX for impact */
	UPROPERTY(EditDefaultsOnly, Category = Effects)
		UParticleSystem* ImpactFX;

	/** Scale the impact FX */
	UPROPERTY(EditDefaultsOnly, Category = Effects)
		FVector ImpactFXScale = FVector(1.f, 1.f, 1.f);

	/** For generating Random numbers*/
	UPROPERTY(BlueprintReadWrite)
		FRandomStream WeaponRandomStream = FRandomStream(0);

	UPROPERTY(EditDefaultsOnly)
		UDebugLineDrawer* DebugLineDrawer;

public:
	/* ------------- */
	/*   Functions   */
	/* ------------- */

	void PreExecuteFire() override;
	void ExecuteFire() override;
	void PostExecuteFire() override;
	void OnActivate() override;

	/* Perform a line trace to retrieve hit info in Shooting direction
	(applySpread: spread will be generate based on a seed - dont use for fx!) */
	FHitResult WeaponTraceShootDirection(bool applySpread);

	/* Perform a line trace to retrieve hit info */
	FHitResult WeaponTrace(const FVector& StartTrace, const FVector& EndTrace);

	/* Perform a box trace to retrieve hit info */
	FHitResult WeaponBoxTrace(const FVector& StartTrace, const FVector& EndTrace, int radius);

	/* Deals Damage if Character was hit*/
	void DealDamage(const FHitResult hitResult, const FVector& ShootDir);


	UFUNCTION(NetMulticast, Reliable)
		void MulticastSpawnInstantWeaponFX(const FHitResult hitResult);

	virtual void SpawnTrailFX(const FHitResult hitResult);
	UParticleSystemComponent* SpawnTrailFX(const FVector& end, bool isFirstPerson);

	void SpawnImpactFX(const FHitResult hitResult);

private:
	bool damageWasDealt = false;
	bool hitWasHeadshot = false;
};
