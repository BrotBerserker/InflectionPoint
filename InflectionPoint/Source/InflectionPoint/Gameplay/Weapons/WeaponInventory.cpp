// Fill out your copyright notice in the Description page of Project Settings.

#include "InflectionPoint.h"
#include "WeaponInventory.h"


// Sets default values for this component's properties
UWeaponInventory::UWeaponInventory() {
	PrimaryComponentTick.bCanEverTick = false;
	SetIsReplicated(true);
}

void UWeaponInventory::GetLifetimeReplicatedProps(TArray< FLifetimeProperty > & OutLifetimeProps) const {
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UWeaponInventory, WeaponSlots);
}

// Called when the game starts
void UWeaponInventory::BeginPlay() {
	Super::BeginPlay();

	if(!GetOwner()->HasAuthority()) {
		return;
	}

	InitDefaultWeapons();
}

void UWeaponInventory::InitDefaultWeapons() {
	AssertTrue(WeaponSlots.Num() > 0, GetWorld(), __FILE__, __LINE__, "Inventory has no Slots assigned!"); // change in blueprint
	for(auto& slot : WeaponSlots) {
		if(!slot.DefaultWeapon)
			continue;
		AddWeapon(slot.SlotType, slot.DefaultWeapon);
	}
}

void UWeaponInventory::Destroy() {
	for(auto& item : WeaponSlots) {
		ClearWeaponSlot(item);
	}
}

void UWeaponInventory::ClearWeaponSlot(FInventoryWeaponSlot slot) {
	if(!slot.Weapon)
		return;
	slot.Weapon->Destroy();
	slot.Weapon = nullptr;
}

void UWeaponInventory::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) {
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

}

bool UWeaponInventory::IsReadyForInitialization() {
	for(auto& item : WeaponSlots) {
		if(item.Weapon && !item.Weapon->IsReadyForInitialization()) {
			return false;
		}
	}
	return true;
}

void UWeaponInventory::AddWeapon(EInventorySlot slot, TSubclassOf<ABaseWeapon> weaponClass) {
	AssertTrue(GetOwner()->HasAuthority(), GetWorld(), __FILE__, __LINE__, "Only call on server");
	int index = GetWeaponSlotIndex(slot);
	AssertTrue(index >= 0, GetWorld(), __FILE__, __LINE__, "Inventory Slot dose not exist!");
	if(WeaponSlots[index].Weapon)
		ClearWeaponSlot(WeaponSlots[index]);
	WeaponSlots[index].Weapon = SpawnWeapon(weaponClass);
}

ABaseWeapon* UWeaponInventory::SpawnWeapon(TSubclassOf<ABaseWeapon> weaponClass) {
	FActorSpawnParameters spawnParams;
	spawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	spawnParams.Instigator = (APawn*)GetOwner();
	spawnParams.Owner = GetOwner();
	return GetWorld()->SpawnActor<ABaseWeapon>(weaponClass, spawnParams);
}

ABaseWeapon* UWeaponInventory::GetNextWeaponInDirection(ABaseWeapon* CurrentWeapon, bool isDirectionForward) {
	int currentIndex = GetWeaponSlotIndex(CurrentWeapon);
	if(currentIndex < 0)
		currentIndex = 0;
	for(int i = 1; i < WeaponSlots.Num() + 1; i++) {
		int modifier = isDirectionForward ? i : i * (-1) + WeaponSlots.Num(); // if moving right or left
		int index = (currentIndex + modifier) % WeaponSlots.Num();
		if(WeaponSlots[index].Weapon != nullptr)
			return WeaponSlots[index].Weapon;
	}
	return CurrentWeapon;
}

ABaseWeapon* UWeaponInventory::GetNextWeapon(ABaseWeapon* CurrentWeapon) {
	return GetNextWeaponInDirection(CurrentWeapon, false);
}

ABaseWeapon* UWeaponInventory::GetPreviousWeapon(ABaseWeapon* CurrentWeapon) {
	return GetNextWeaponInDirection(CurrentWeapon, true);
}

ABaseWeapon* UWeaponInventory::GetNextUsableWeapon(ABaseWeapon* CurrentWeapon) {
	auto nextWeapon = GetNextWeapon(CurrentWeapon);
	for(int i = 0; i < WeaponSlots.Num(); i++) {
		if(nextWeapon->CurrentAmmo != 0)
			return nextWeapon;
		nextWeapon = GetNextWeapon(CurrentWeapon);
	}
	return CurrentWeapon;
}

ABaseWeapon* UWeaponInventory::GetPreviousUsableWeapon(ABaseWeapon* CurrentWeapon) {
	auto nextWeapon = GetPreviousWeapon(CurrentWeapon);
	for(int i = 0; i < WeaponSlots.Num(); i++) {
		if(nextWeapon->CurrentAmmo != 0)
			return nextWeapon;
		nextWeapon = GetPreviousWeapon(CurrentWeapon);
	}
	return CurrentWeapon;
}

ABaseWeapon* UWeaponInventory::GetRandomWeapon() {
	int index = FMath::RandHelper(WeaponSlots.Num());
	for(int i = 1; i < WeaponSlots.Num(); i++) {
		int nextIndex = (index + i) % WeaponSlots.Num();
		auto slot = WeaponSlots[nextIndex];
		if(!slot.Weapon || slot.Weapon->CurrentAmmo == 0)
			continue;
		return slot.Weapon;
	}
	return NULL;
}

ABaseWeapon* UWeaponInventory::GetWeapon(EInventorySlot slot) {
	int index = GetWeaponSlotIndex(slot);
	if(index < 0)
		return NULL;
	return WeaponSlots[index].Weapon;
}

int UWeaponInventory::GetWeaponNum() {
	return WeaponSlots.Num();
}

ABaseWeapon* UWeaponInventory::GetWeaponByClass(UClass* weaponClass) {
	for(auto& item : WeaponSlots)
		if(item.Weapon && item.Weapon->IsA(weaponClass))
			return item.Weapon;
	return NULL;
}

int UWeaponInventory::GetWeaponSlotIndex(ABaseWeapon* weapon) {
	for(int i = 0; i < WeaponSlots.Num(); i++) {
		if(WeaponSlots[i].Weapon == weapon)
			return i;
	}
	return -1;
}

int UWeaponInventory::GetWeaponSlotIndex(EInventorySlot type) {
	for(int i = 0; i < WeaponSlots.Num(); i++) {
		if(WeaponSlots[i].SlotType == type)
			return i;
	}
	return -1;
}