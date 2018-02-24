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
		float Spread;

	/** weapon range */
	UPROPERTY(EditDefaultsOnly, Category = WeaponConfig)
		float Range;

	/** damage amount */
	UPROPERTY(EditDefaultsOnly, Category = WeaponConfig)
		int32 Damage;

	/** type of damage */
	UPROPERTY(EditDefaultsOnly, Category = WeaponConfig)
		TSubclassOf<UDamageType> DamageType;

	/** smoke trail */
	UPROPERTY(EditDefaultsOnly, Category = Effects)
		UParticleSystem* TrailFX;

	/** param name for beam target in smoke trail */
	UPROPERTY(EditDefaultsOnly, Category = Effects)
		FName TrailTargetParam;

	/** FX for muzzle flash */
	UPROPERTY(EditDefaultsOnly, Category = Effects)
		UParticleSystem* MuzzleFX;

	/** FX for muzzle flash */
	UPROPERTY(EditDefaultsOnly, Category = Effects)
		bool IsMuzzleFXLooped = true;

public:
		void ExecuteFire() override;
		FHitResult AInstantWeapon::WeaponTrace(const FVector& StartTrace, const FVector& EndTrace);
		void DealDamage(const FHitResult& Impact, const FVector& ShootDir);

		UFUNCTION(BlueprintCallable, NetMulticast, Unreliable)
			void MulticastSpawnWeaponEffects(const FVector& startPoint,const FVector& endPoint);

		void SpawnMuzzleFX();
		void SpawnTrailFX(const FVector& startPoint, const FVector& endPoint);
		void SpawnImpactFX();

		UFUNCTION()
			void DecativateParticleSystem(UParticleSystemComponent* effect);

};
