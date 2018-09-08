// Fill out your copyright notice in the Description page of Project Settings.

#include "InflectionPoint.h"
#include "WeaponInventory.h"


// Sets default values for this component's properties
UWeaponInventory::UWeaponInventory() {
	PrimaryComponentTick.bCanEverTick = false;
}

// Called when the game starts
void UWeaponInventory::BeginPlay() {
	Super::BeginPlay();

	if(!GetOwner()->HasAuthority()) {
		return;
	}

	for(auto item : DefaultWeaponClasses) {
		FActorSpawnParameters spawnParams;
		spawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		spawnParams.Instigator = (APawn*) GetOwner();
		spawnParams.Owner = GetOwner();
		ABaseWeapon* spawnedWeapon = GetWorld()->SpawnActor<ABaseWeapon>(item, spawnParams);
		weapons.AddUnique(spawnedWeapon);
	}
}

void UWeaponInventory::Destroy() {
	for(ABaseWeapon* weapon : weapons) {
		weapon->Destroy();
	}
}

// Called every frame
void UWeaponInventory::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) {
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

bool UWeaponInventory::IsReadyForInitialization() {
	for(ABaseWeapon* weapon : weapons) {
		if(!weapon->IsReadyForInitialization()) {
			return false;
		}
	}
	return true;
}

ABaseWeapon* UWeaponInventory::GetNextUsableWeapon(ABaseWeapon* CurrentWeapon) {
	int32 index = weapons.IndexOfByKey(CurrentWeapon);
	for(int i = 1; i < weapons.Num(); i++) {
		auto weapon = weapons[(index + i) % weapons.Num()];
		if(IsWeaponUsable(weapon->GetClass()))
			return weapon;
	}
	return CurrentWeapon;
}

ABaseWeapon* UWeaponInventory::GetPreviousUsableWeapon(ABaseWeapon* CurrentWeapon) {
	int32 index = weapons.IndexOfByKey(CurrentWeapon);
	for(int i = 1; i < weapons.Num(); i++) {
		auto weapon = weapons[(index - i + weapons.Num()) % weapons.Num()];
		if(IsWeaponUsable(weapon->GetClass()))
			return weapon;
	}
	return CurrentWeapon;
}

ABaseWeapon* UWeaponInventory::GetWeapon(int index) {
	if(index < 0 || index >= weapons.Num())
		return NULL;
	return weapons[index];
}

ABaseWeapon* UWeaponInventory::GetRandomWeapon() {
	int32 index = FMath::RandHelper(weapons.Num());
	for(int i = 1; i < weapons.Num(); i++) {
		auto weapon = weapons[(index + i ) % weapons.Num()];
		if(IsWeaponUsable(weapon->GetClass()))
			return weapon;
	}
	return NULL;
}

int UWeaponInventory::GetWeaponNum() {
	return weapons.Num();
}

ABaseWeapon* UWeaponInventory::GetWeaponByClass(UClass* weaponClass) {
	for(ABaseWeapon* weapon : weapons)
		if(weapon->IsA(weaponClass))
			return weapon;
	return NULL;
}

bool UWeaponInventory::IsWeaponUsable(UClass* weaponClass) {
	return UsableWeapons.Contains(weaponClass);
}

void UWeaponInventory::SetWeaponUsabilityStatus(UClass* weaponClass, bool usable) {
	if(usable && !IsWeaponUsable(weaponClass))
		UsableWeapons.Add(weaponClass);
	if(!usable && IsWeaponUsable(weaponClass))
		UsableWeapons.Remove(weaponClass);
}