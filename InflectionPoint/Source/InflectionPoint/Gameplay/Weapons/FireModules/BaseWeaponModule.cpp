// Fill out your copyright notice in the Description page of Project Settings.

#include "InflectionPoint.h"
#include "Gameplay/Characters/BaseCharacter.h"
#include "Gameplay/Weapons/BaseWeapon.h"
#include "BaseWeaponModule.h"

UBaseWeaponModule::UBaseWeaponModule() {
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = true;
	SetIsReplicated(true); // wtf junge warum ist ihm das egal und man muss es trotzdem noch im blueprint anhaken?!
}

void UBaseWeaponModule::GetLifetimeReplicatedProps(TArray< FLifetimeProperty > & OutLifetimeProps) const {
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UBaseWeaponModule, Weapon);
	DOREPLIFETIME(UBaseWeaponModule, OwningCharacter);
	DOREPLIFETIME(UBaseWeaponModule, CurrentState);
	DOREPLIFETIME(UBaseWeaponModule, FireMode);
}

void UBaseWeaponModule::EndPlay(const EEndPlayReason::Type EndPlayReason) {
	if(FireLoopSoundComponent)
		FireLoopSoundComponent->DestroyComponent();
	if(ChargeSoundComponent)
		ChargeSoundComponent->DestroyComponent();
	if(CurrentState == EWeaponModuleState::FIRING) {
		StopFire();
	}
}

void UBaseWeaponModule::TickComponent(float DeltaTime, enum ELevelTick tickType, FActorComponentTickFunction *thisTickFunction) {
	Super::TickComponent(DeltaTime, tickType, thisTickFunction);
	Weapon->TogglePersistentSoundFX(FireLoopSoundComponent, FireLoopSound, CurrentState == EWeaponModuleState::FIRING);
	Weapon->TogglePersistentSoundFX(ChargeSoundComponent, ChargeSound, CurrentState == EWeaponModuleState::CHARGING);

	if(!GetOwner() || !GetOwner()->HasAuthority()) {
		return;
	}

	timeSinceLastShot += DeltaTime;
	timeSinceStartFire += DeltaTime;

	if(CurrentState == EWeaponModuleState::CHARGING && timeSinceStartFire >= ChargeDuration) {
		ChangeModuleState(EWeaponModuleState::FIRING);
	} else if(!OwningCharacter->IsAReplay() && CurrentState == EWeaponModuleState::FIRING && timeSinceLastShot >= FireInterval) {
		Fire();
	}
}

void UBaseWeaponModule::FireOnce() {
	if(Weapon->CurrentAmmo == 0 && Weapon->CurrentAmmoInClip == 0) {
		Weapon->MulticastSpawnWeaponSound(NoAmmoSound);
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
	Weapon->TogglePersistentSoundFX(FireLoopSoundComponent, FireLoopSound, false);
	Weapon->TogglePersistentSoundFX(ChargeSoundComponent, ChargeSound, false);
	if(CurrentState != EWeaponModuleState::DEACTIVATED)
		ChangeModuleState(EWeaponModuleState::IDLE);
}

bool UBaseWeaponModule::StartFire() {
	wantsToFire = true;
	timeSinceStartFire = 0;
	if(CurrentState == EWeaponModuleState::DEACTIVATED)
		return false;
	if(Weapon->CurrentAmmo == 0 && Weapon->CurrentAmmoInClip == 0)
		Weapon->MulticastSpawnWeaponSound(NoAmmoSound);
	if(CurrentState == EWeaponModuleState::IDLE && Weapon->CurrentAmmoInClip > 0) {
		ChangeModuleState(EWeaponModuleState::CHARGING);
		return true;
	}
	return false;
}

bool UBaseWeaponModule::CanFire() {
	return true;
}

bool UBaseWeaponModule::IsFiring() {
	return CurrentState == EWeaponModuleState::FIRING || CurrentState == EWeaponModuleState::CHARGING;
}

void UBaseWeaponModule::Fire() {
	if(CanFire()) {
		Weapon->RecordModuleFired(FireMode);
		if(Weapon->CurrentAmmoInClip <= 0)
			return;
		timeSinceLastShot = 0;
		PreExecuteFire();
		for(int i = 0; i < FireShotNum; i++)
			ExecuteFire();
		PostExecuteFire();
		Weapon->CurrentAmmoInClip--;
		if(Weapon->CurrentAmmo > 0)
			Weapon->CurrentAmmo--;
		Weapon->ForceNetUpdate();
		FireExecuted();
	}
	if(!AutoFire)
		ChangeModuleState(EWeaponModuleState::IDLE);
}

void UBaseWeaponModule::FireExecuted() {
	MulticastPlayCameraShake();
	Weapon->MulticastSpawnMuzzleFX(MuzzleFX, MuzzleFXDuration, MuzzleFXScale);
	Weapon->MulticastSpawnWeaponSound(FireSound);
	Weapon->MulticastPlayFireAnimation();
}

void UBaseWeaponModule::MulticastPlayCameraShake_Implementation() {
	if(OwningCharacter && Cast<APlayerController>(OwningCharacter->GetController())) {
		Cast<APlayerController>(OwningCharacter->GetController())->PlayerCameraManager->PlayCameraShake(FireCameraShake, 1.0f);
	} 
}

void UBaseWeaponModule::ChangeModuleState(EWeaponModuleState newState) {
	CurrentState = newState;
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