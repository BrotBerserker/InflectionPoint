// Fill out your copyright notice in the Description page of Project Settings.

#include "InflectionPoint.h"
#include "Gameplay/Characters/BaseCharacter.h"
#include "Gameplay/Weapons/BaseWeapon.h"
#include "AimingWeaponModule.h"


void UAimingWeaponModule::StartFire() {
	Super::StartFire();
	OwningCharacter->StartAiming();
}

void UAimingWeaponModule::StopFire() {
	Super::StopFire();
	OwningCharacter->StopAiming();
}