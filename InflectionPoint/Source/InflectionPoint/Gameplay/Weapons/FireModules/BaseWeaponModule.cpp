// Fill out your copyright notice in the Description page of Project Settings.

#include "InflectionPoint.h"
#include "Gameplay/Characters/BaseCharacter.h"
#include "Gameplay/Weapons/BaseWeapon.h"
#include "BaseWeaponModule.h"


void UBaseWeaponModule::Initialize() {}

void UBaseWeaponModule::Dispose() {
	if(FireLoopSoundComponent)
		FireLoopSoundComponent->DestroyComponent();
	if(ChargeSoundComponent)
		ChargeSoundComponent->DestroyComponent();
	StopFire();
}

void UBaseWeaponModule::AuthorityTick(float DeltaTime) {
	timeSinceLastShot += DeltaTime;
	timeSinceStartFire += DeltaTime;

	if(CurrentState == EWeaponModuleState::CHARGING && timeSinceStartFire >= ChargeDuration) {
		ChangeModuleState(EWeaponModuleState::FIRING);
	} else if(CurrentState == EWeaponModuleState::FIRING && timeSinceLastShot >= FireInterval) {
		Fire();
		//} else if(Recorder && RecordKeyReleaseNextTick) {
		//	RecordKeyReleaseNextTick = false;
		//	Recorder->ServerRecordKeyReleased("WeaponFired");
	}
	// You can not only take the CurrentState because of replays only calling FireOnce()
	shouldPlayFireFX = shouldPlayFireFX && timeSinceLastShot <= FireInterval + 0.1;
}

void UBaseWeaponModule::Tick(float DeltaTime) {
	Weapon->TogglePersistentSoundFX(FireLoopSoundComponent, FireLoopSound, shouldPlayFireFX);
	Weapon->TogglePersistentSoundFX(ChargeSoundComponent, ChargeSound, CurrentState == EWeaponModuleState::CHARGING);
}

UWorld* UBaseWeaponModule::GetWorld() const {
	return GetOuter()->GetWorld();
}

void UBaseWeaponModule::FireOnce() {
	if(Weapon->CurrentAmmo == 0 && Weapon->CurrentAmmoInClip == 0) {
		Weapon->MulticastSpawnNoAmmoSound();
		return;
	}
	if(CurrentState != EWeaponModuleState::IDLE || timeSinceLastShot < FireInterval)
		return;
	ChangeModuleState(EWeaponModuleState::FIRING); // No charging for replays
	Fire();
	ChangeModuleState(EWeaponModuleState::IDLE);
}

void UBaseWeaponModule::StopFire() {
	wantsToFire = false;
	shouldPlayFireFX = false;
	Weapon->TogglePersistentSoundFX(FireLoopSoundComponent, FireLoopSound, false);
	Weapon->TogglePersistentSoundFX(ChargeSoundComponent, ChargeSound, false);
	ChangeModuleState(EWeaponModuleState::IDLE);
}

void UBaseWeaponModule::StartFire() {
	wantsToFire = true;
	timeSinceStartFire = 0;
	if(Weapon->CurrentAmmo == 0 && Weapon->CurrentAmmoInClip == 0) 
		Weapon->MulticastSpawnNoAmmoSound();
	if(CurrentState == EWeaponModuleState::IDLE && Weapon->CurrentAmmoInClip > 0)
		ChangeModuleState(EWeaponModuleState::CHARGING);
}

bool UBaseWeaponModule::CanFire() {
	return true;
}

bool UBaseWeaponModule::IsFireing() {
	return CurrentState == EWeaponModuleState::FIRING || CurrentState == EWeaponModuleState::CHARGING;
}

void UBaseWeaponModule::Fire() {
	if(CanFire()) {
		//if(Recorder) {
		//	RecordKeyReleaseNextTick = true;
		//	Recorder->ServerRecordKeyPressed("WeaponFired");
		//}
		if(Weapon->CurrentAmmoInClip <= 0)
			return;
		shouldPlayFireFX = true;
		timeSinceLastShot = 0;
		PreExecuteFire();
		for(int i = 0; i < FireShotNum; i++)
			ExecuteFire();
		PostExecuteFire();
		Weapon->CurrentAmmoInClip--;
		Weapon->CurrentAmmo--;
		Weapon->ForceNetUpdate();
		MulticastFireExecuted();
	}
	if(!AutoFire)
		ChangeModuleState(EWeaponModuleState::IDLE);
}

void UBaseWeaponModule::MulticastFireExecuted_Implementation() {
	if(OwningCharacter && Cast<APlayerController>(OwningCharacter->GetController()))
		Cast<APlayerController>(OwningCharacter->GetController())->PlayerCameraManager->PlayCameraShake(FireCameraShake, 1.0f);
	Weapon->SpawnMuzzleFX(MuzzleFX, MuzzleFXDuration, MuzzleFXScale);
	Weapon->SpawnWeaponSound(FireSound);
	Weapon->PlayFireAnimation();
	//OnFireExecuted.Broadcast();
}


void UBaseWeaponModule::ChangeModuleState(EWeaponModuleState newState) {
	CurrentState = newState;
	//MulticastStateChanged(newState); // todo
}

void UBaseWeaponModule::PreExecuteFire() {}
void UBaseWeaponModule::PostExecuteFire() {}

void UBaseWeaponModule::OnActivate() {
	timeSinceLastShot = FireInterval; // so you can fire after activation
	if(wantsToFire) {
		timeSinceStartFire = 0;
		ChangeModuleState(EWeaponModuleState::CHARGING);
	} else {
		ChangeModuleState(EWeaponModuleState::IDLE);
	}
}
void UBaseWeaponModule::OnDeactivate() {
	ChangeModuleState(EWeaponModuleState::DEACTIVATED);
}