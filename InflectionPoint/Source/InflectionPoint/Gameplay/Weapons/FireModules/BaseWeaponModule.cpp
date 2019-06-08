// Fill out your copyright notice in the Description page of Project Settings.

#include "InflectionPoint.h"
#include "Gameplay/Characters/BaseCharacter.h"
#include "Gameplay/Weapons/BaseWeapon.h"
#include "BaseWeaponModule.h"


bool UBaseWeaponModule::CanFire() {
	return true;
}

void UBaseWeaponModule::Fire() {
	if(CanFire()) {
		PreExecuteFire();
		for(int i = 0; i < FireShotNum; i++)
			ExecuteFire();
		PostExecuteFire();
	}
}

TStatId UBaseWeaponModule::GetStatId() const {
	return Super::GetStatID(); 
}
//
//UWorld* UBaseWeaponModule::GetWorld() const {
//	return GetOuter()->GetWorld();
//}

void UBaseWeaponModule::PreExecuteFire() {}
void UBaseWeaponModule::PostExecuteFire() {}

void UBaseWeaponModule::OnActivate() {}
void UBaseWeaponModule::OnDeactivate() {}