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

	AProjectileWeapon();

	/** Projectile class to spawn */
	UPROPERTY(EditDefaultsOnly, Category = Projectile)
		TSubclassOf<class AInflectionPointProjectile> ProjectileClass;

	void ExecuteFire() override;

};
