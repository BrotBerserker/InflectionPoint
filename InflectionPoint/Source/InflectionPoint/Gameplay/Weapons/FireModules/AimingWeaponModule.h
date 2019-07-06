// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Gameplay/Weapons/FireModules/BaseWeaponModule.h"
#include "AimingWeaponModule.generated.h"

/**
 * 
 */
UCLASS()
class INFLECTIONPOINT_API UAimingWeaponModule : public UBaseWeaponModule
{
	GENERATED_BODY()
public:
	virtual void Fire() override {}; // do nothing on fire
	virtual void StartFire() override;
	virtual void StopFire() override;
};
