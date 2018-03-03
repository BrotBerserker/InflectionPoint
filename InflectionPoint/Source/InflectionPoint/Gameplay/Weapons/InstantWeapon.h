// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Gameplay/Weapons/BaseWeapon.h"
#include "Gameplay/Characters/BaseCharacter.h"
#include "InstantWeapon.generated.h"

/**
 *
 */
UCLASS()
class INFLECTIONPOINT_API AInstantWeapon : public ABaseWeapon {
	GENERATED_BODY()

public:
	/** base weapon spread (degrees) */
	UPROPERTY(EditDefaultsOnly, Category = WeaponConfig)
		float Spread = 0;

	/** weapon range */
	UPROPERTY(EditDefaultsOnly, Category = WeaponConfig)
		float Range = 10000;

	/** damage amount */
	UPROPERTY(EditDefaultsOnly, Category = WeaponConfig)
		int32 Damage = 10;

	/** type of damage */
	UPROPERTY(EditDefaultsOnly, Category = WeaponConfig)
		TSubclassOf<UDamageType> DamageType;

	/** Collision Channel for the raycast*/
	UPROPERTY(EditAnywhere, meta = (Bitmask, BitmaskEnum = "ECollisionChannel"))
		int32 CollisionChannel;

	/** smoke trail */
	UPROPERTY(EditDefaultsOnly, Category = Effects)
		UParticleSystem* TrailFX;

	/** param name for beam target in smoke trail */
	UPROPERTY(EditDefaultsOnly, Category = Effects)
		FName TrailTargetParamName;
	
	/** FX for impact */
	UPROPERTY(EditDefaultsOnly, Category = Effects)
		UParticleSystem* ImpactFX;
	
	/** For generating Random numbers*/
	UPROPERTY(BlueprintReadWrite)
		FRandomStream WeaponRandomStream = FRandomStream(0);

	/* Color for Player debug Trace*/
	UPROPERTY(EditAnywhere, Category = Debug)
		FColor PlayerDebugColor = FColor(10, 12, 160);

	/* Color for Replay debug Trace*/
	UPROPERTY(EditAnywhere, Category = Debug)
		FColor ReplayDebugColor = FColor(160, 14, 0);

public:
	void ExecuteFire() override;

	/* Perform a line trace to retrieve hit info */
	FHitResult AInstantWeapon::WeaponTrace(const FVector& StartTrace, const FVector& EndTrace);

	/* Deals Damage if Character was hit*/
	void DealDamage(const FHitResult hitResult, const FVector& ShootDir);

	void OnEquip() override;

	UFUNCTION(NetMulticast, Reliable)
		void MulticastSpawnInstantWeaponFX(const FHitResult hitResult);

	void SpawnTrailFX(const FHitResult hitResult);

	void SpawnImpactFX(const FHitResult hitResult);

private:
	void DrawDebugLineTrace(const FHitResult hitResult);
};
