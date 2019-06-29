// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Gameplay/Weapons/FireModules/InstantWeaponModule.h"
#include "LaserWeaponModule.generated.h"

/**
 * 
 */
UCLASS()
class INFLECTIONPOINT_API ULaserWeaponModule : public UInstantWeaponModule
{
	GENERATED_BODY()
public:
	virtual void Tick(float DeltaTime) override;

	virtual void SpawnInstantWeaponFX(const FHitResult hitResult) override;

	void SpawnLaserFX();

	void SpawnLaserTrailFX(const FHitResult hitResult);

	void UpdateLaserTrailFX(UParticleSystemComponent* trail, const FVector& start, const FVector& end);

	UParticleSystemComponent* fpLaserTrail;
	UParticleSystemComponent* tpLaserTrail;
	UAudioComponent* LoopedFireSound;
	
};
