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
		FName TrailTargetParam;

	/** FX for muzzle flash */
	UPROPERTY(EditDefaultsOnly, Category = Effects)
		UParticleSystem* MuzzleFX;

	/** FX for impact */
	UPROPERTY(EditDefaultsOnly, Category = Effects)
		UParticleSystem* ImpactFX;

	/** Duration for the muzzle FX flash (-1 for endless)*/
	UPROPERTY(EditDefaultsOnly, Category = Effects)
		float MuzzleFXDuration = 0.1;


	/** For generating Random numbers*/
	UPROPERTY(BlueprintReadWrite)
		FRandomStream WeaponRandomStream = FRandomStream(0);

	UPROPERTY(EditAnywhere)
		FColor DebugColor = FColor(160,60,50);

public:
		void ExecuteFire() override;
		FHitResult AInstantWeapon::WeaponTrace(const FVector& StartTrace, const FVector& EndTrace);
		void DealDamage(const FHitResult& Impact, const FVector& ShootDir);

		UFUNCTION(BlueprintCallable, NetMulticast, Unreliable)
			void MulticastSpawnWeaponEffects(FHitResult hitResult);

		void SpawnMuzzleFX();
		void SpawnTrailFX(const FVector& endPoint);
		void SpawnImpactFX(FHitResult hitResult);
		
		UFUNCTION()
			void DecativateParticleSystem(UParticleSystemComponent* effect);
private:
	void DrawDebugLineTrace(const FVector& endPoint);
};
