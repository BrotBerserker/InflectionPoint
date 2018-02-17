// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Gameplay/Weapons/BaseWeapon.h"
#include "ProjectileWeapon.generated.h"

/**
 *
 */
UCLASS()
class INFLECTIONPOINT_API AProjectileWeapon : public ABaseWeapon {
	GENERATED_BODY()

public:

	/** Projectile class to spawn */
	UPROPERTY(EditDefaultsOnly, Category = Projectile)
		TSubclassOf<class AInflectionPointProjectile> ProjectileClass;

	/** Returns the location at which a projectile should spawn */
	FVector GetProjectileSpawnLocation();

	/** Returns the rotation with which a projectile should spawn */
	FRotator GetProjectileSpawnRotation();

	void ExecuteFire() override;

};
