// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Gameplay/Weapons/InstantWeapon.h"
#include "Gameplay/Weapons/BaseWeapon.h"
#include "LaserWeapon.generated.h"

/**
 * 
 */
UCLASS()
class INFLECTIONPOINT_API ALaserWeapon : public AInstantWeapon
{
	GENERATED_BODY()
public:

	virtual void Tick(float DeltaTime) override;

	void MulticastSpawnInstantWeaponFX_Implementation(const FHitResult hitResult) override;

	void SpawnLaserFX();

	void SpawnTrailFX(const FHitResult hitResult) override;

	void UpdateTrailFX(UParticleSystemComponent* trail, const FVector& start, const FVector& end);

	UParticleSystemComponent* fpLaserTrail;
	UParticleSystemComponent* tpLaserTrail;
	UAudioComponent* LoopedFireSound;
};
