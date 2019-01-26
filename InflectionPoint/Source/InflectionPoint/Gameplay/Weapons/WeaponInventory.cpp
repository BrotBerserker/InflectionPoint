// Fill out your copyright notice in the Description page of Project Settings.

#include "InflectionPoint.h"
#include "Gameplay/Characters/BaseCharacter.h"
#include "Gamemodes/TDMPlayerStateBase.h"
#include "WeaponInventory.h"


// Sets default values for this component's properties
UWeaponInventory::UWeaponInventory() {DebugPrint(__FILE__, __LINE__);DebugPrint(__FILE__, __LINE__);
	PrimaryComponentTick.bCanEverTick = false;
	SetIsReplicated(true);
DebugPrint(__FILE__, __LINE__);}

void UWeaponInventory::GetLifetimeReplicatedProps(TArray< FLifetimeProperty > & OutLifetimeProps) const {DebugPrint(__FILE__, __LINE__);DebugPrint(__FILE__, __LINE__);
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UWeaponInventory, WeaponSlots);
DebugPrint(__FILE__, __LINE__);}

// Called when the game starts
void UWeaponInventory::BeginPlay() {DebugPrint(__FILE__, __LINE__);DebugPrint(__FILE__, __LINE__);
	Super::BeginPlay();

	if(!GetOwner()->HasAuthority()) {DebugPrint(__FILE__, __LINE__);
		return;
	DebugPrint(__FILE__, __LINE__);}

	InitDefaultWeapons();
DebugPrint(__FILE__, __LINE__);}

void UWeaponInventory::InitDefaultWeapons() {DebugPrint(__FILE__, __LINE__);
	AssertTrue(WeaponSlots.Num() > 0, GetWorld(), __FILE__, __LINE__, "Inventory has no Slots assigned!"); // change in blueprint
	for(auto& slot : WeaponSlots) {DebugPrint(__FILE__, __LINE__);
		if(slot.EditorOnlyDefaultWeapon && GetWorld()->WorldType == EWorldType::PIE) {DebugPrint(__FILE__, __LINE__);
			SetWeaponAtPosition(slot.SlotPosition, slot.EditorOnlyDefaultWeapon);
			continue;
		DebugPrint(__FILE__, __LINE__);}
		if(!slot.DefaultWeapon)
			continue;
		SetWeaponAtPosition(slot.SlotPosition, slot.DefaultWeapon);
	DebugPrint(__FILE__, __LINE__);}
DebugPrint(__FILE__, __LINE__);}

bool UWeaponInventory::ServerDestroy_Validate() {DebugPrint(__FILE__, __LINE__);
	return true; 
DebugPrint(__FILE__, __LINE__);}

void UWeaponInventory::ServerDestroy_Implementation() {DebugPrint(__FILE__, __LINE__);
	for(auto& item : WeaponSlots) {DebugPrint(__FILE__, __LINE__);
		ClearWeaponSlot(item);
	DebugPrint(__FILE__, __LINE__);}
DebugPrint(__FILE__, __LINE__);}

void UWeaponInventory::ClearWeaponSlot(FInventoryWeaponSlot slot) {DebugPrint(__FILE__, __LINE__);
	if(!slot.Weapon)
		return;
	slot.Weapon->Destroy();
	slot.Weapon = nullptr;
DebugPrint(__FILE__, __LINE__);}

bool UWeaponInventory::IsReadyForInitialization() {DebugPrint(__FILE__, __LINE__);
	for(auto& item : WeaponSlots) {DebugPrint(__FILE__, __LINE__);
		if(item.Weapon && !item.Weapon->IsReadyForInitialization()) {DebugPrint(__FILE__, __LINE__);
			return false;
		DebugPrint(__FILE__, __LINE__);}
	DebugPrint(__FILE__, __LINE__);}
	return true;
DebugPrint(__FILE__, __LINE__);}

void UWeaponInventory::SetWeaponAtPosition(EInventorySlotPosition position, TSubclassOf<ABaseWeapon> weaponClass) {DebugPrint(__FILE__, __LINE__);
	AssertTrue(GetOwner()->HasAuthority(), GetWorld(), __FILE__, __LINE__, "Only call on server");
	int index = GetWeaponSlotIndex(position);
	AssertTrue(index >= 0, GetWorld(), __FILE__, __LINE__, "Inventory Slot dose not exist!");
	auto weapon = GetWeaponByClass(weaponClass);
	if(weapon) {DebugPrint(__FILE__, __LINE__);
		// move weapon to new slot
		int index2 = GetWeaponSlotIndex(weapon);
		WeaponSlots[index2].Weapon = nullptr;
	DebugPrint(__FILE__, __LINE__);} else {DebugPrint(__FILE__, __LINE__);
		// create new weapon
		weapon = SpawnWeapon(weaponClass);
	DebugPrint(__FILE__, __LINE__);}
	// remove old weapon from slot
	if(WeaponSlots[index].Weapon)
		ClearWeaponSlot(WeaponSlots[index]);
	WeaponSlots[index].Weapon = weapon;
DebugPrint(__FILE__, __LINE__);}

ABaseWeapon* UWeaponInventory::SpawnWeapon(TSubclassOf<ABaseWeapon> weaponClass) {DebugPrint(__FILE__, __LINE__);
	FActorSpawnParameters spawnParams;
	spawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	spawnParams.Instigator = (APawn*)GetOwner();
	spawnParams.Owner = GetOwner();
	return GetWorld()->SpawnActor<ABaseWeapon>(weaponClass, spawnParams);
DebugPrint(__FILE__, __LINE__);}

ABaseWeapon* UWeaponInventory::GetNextWeaponInDirection(ABaseWeapon* CurrentWeapon, bool isDirectionForward) {DebugPrint(__FILE__, __LINE__);
	int currentIndex = GetWeaponSlotIndex(CurrentWeapon);
	if(currentIndex < 0)
		currentIndex = 0;
	int startOffset = CurrentWeapon ? 1 : 0; // if weapon null start with first weapon
	for(int i = startOffset; i < WeaponSlots.Num() + 1; i++) {DebugPrint(__FILE__, __LINE__);
		int modifier = isDirectionForward ? i : i * (-1); // if moving right or left
		int index = (currentIndex + modifier + WeaponSlots.Num()) % WeaponSlots.Num();
		if(WeaponSlots[index].Weapon != nullptr)
			return WeaponSlots[index].Weapon;
	DebugPrint(__FILE__, __LINE__);}
	return CurrentWeapon;
DebugPrint(__FILE__, __LINE__);}

ABaseWeapon* UWeaponInventory::GetNextWeapon(ABaseWeapon* CurrentWeapon) {DebugPrint(__FILE__, __LINE__);
	return GetNextWeaponInDirection(CurrentWeapon, true);
DebugPrint(__FILE__, __LINE__);}

ABaseWeapon* UWeaponInventory::GetPreviousWeapon(ABaseWeapon* CurrentWeapon) {DebugPrint(__FILE__, __LINE__);
	return GetNextWeaponInDirection(CurrentWeapon, false);
DebugPrint(__FILE__, __LINE__);}

ABaseWeapon* UWeaponInventory::GetNextUsableWeapon(ABaseWeapon* CurrentWeapon) {DebugPrint(__FILE__, __LINE__);
	auto nextWeapon = GetNextWeapon(CurrentWeapon);
	for(int i = 0; nextWeapon && i < WeaponSlots.Num(); i++) {DebugPrint(__FILE__, __LINE__);
		if(nextWeapon->CurrentAmmo != 0)
			return nextWeapon;
		nextWeapon = GetNextWeapon(nextWeapon);
	DebugPrint(__FILE__, __LINE__);}
	return CurrentWeapon;
DebugPrint(__FILE__, __LINE__);}

ABaseWeapon* UWeaponInventory::GetPreviousUsableWeapon(ABaseWeapon* CurrentWeapon) {DebugPrint(__FILE__, __LINE__);
	auto nextWeapon = GetPreviousWeapon(CurrentWeapon);
	for(int i = 0; nextWeapon && i < WeaponSlots.Num(); i++) {DebugPrint(__FILE__, __LINE__);
		if(nextWeapon->CurrentAmmo != 0)
			return nextWeapon;
		nextWeapon = GetPreviousWeapon(nextWeapon);
	DebugPrint(__FILE__, __LINE__);}
	return CurrentWeapon;
DebugPrint(__FILE__, __LINE__);}

ABaseWeapon* UWeaponInventory::GetRandomWeapon() {DebugPrint(__FILE__, __LINE__);
	int index = FMath::RandHelper(WeaponSlots.Num());
	for(int i = 0; i < WeaponSlots.Num(); i++) {DebugPrint(__FILE__, __LINE__);
		int nextIndex = (index + i) % WeaponSlots.Num();
		auto slot = WeaponSlots[nextIndex];
		if(!slot.Weapon || slot.Weapon->CurrentAmmo == 0)
			continue;
		return slot.Weapon;
	DebugPrint(__FILE__, __LINE__);}
	return NULL;
DebugPrint(__FILE__, __LINE__);}

ABaseWeapon* UWeaponInventory::GetWeapon(EInventorySlotPosition slot) {DebugPrint(__FILE__, __LINE__);
	int index = GetWeaponSlotIndex(slot);
	if(index < 0)
		return NULL;
	return WeaponSlots[index].Weapon;
DebugPrint(__FILE__, __LINE__);}

int UWeaponInventory::GetWeaponNum() {DebugPrint(__FILE__, __LINE__);
	return WeaponSlots.Num();
DebugPrint(__FILE__, __LINE__);}

ABaseWeapon* UWeaponInventory::GetWeaponByClass(UClass* weaponClass) {DebugPrint(__FILE__, __LINE__);
	for(auto& item : WeaponSlots)
		if(item.Weapon && item.Weapon->IsA(weaponClass))
			return item.Weapon;
	return NULL;
DebugPrint(__FILE__, __LINE__);}

int UWeaponInventory::GetWeaponSlotIndex(ABaseWeapon* weapon) {DebugPrint(__FILE__, __LINE__);
	for(int i = 0; weapon && i < WeaponSlots.Num(); i++) {DebugPrint(__FILE__, __LINE__);
		if(WeaponSlots[i].Weapon == weapon)
			return i;
	DebugPrint(__FILE__, __LINE__);}
	return -1;
DebugPrint(__FILE__, __LINE__);}

int UWeaponInventory::GetWeaponSlotIndex(EInventorySlotPosition type) {DebugPrint(__FILE__, __LINE__);
	for(int i = 0; i < WeaponSlots.Num(); i++) {DebugPrint(__FILE__, __LINE__);
		if(WeaponSlots[i].SlotPosition == type)
			return i;
	DebugPrint(__FILE__, __LINE__);}
	return -1;
DebugPrint(__FILE__, __LINE__);}