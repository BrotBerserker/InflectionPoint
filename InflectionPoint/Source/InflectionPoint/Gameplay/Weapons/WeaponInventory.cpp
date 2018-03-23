// Fill out your copyright notice in the Description page of Project Settings.

#include "InflectionPoint.h"
#include "WeaponInventory.h"


// Sets default values for this component's properties
UWeaponInventory::UWeaponInventory() {DebugPrint(__FILE__, __LINE__);
	PrimaryComponentTick.bCanEverTick = false;
DebugPrint(__FILE__, __LINE__);}

// Called when the game starts
void UWeaponInventory::BeginPlay() {DebugPrint(__FILE__, __LINE__);
	Super::BeginPlay();

	if(!GetOwner()->HasAuthority()) {DebugPrint(__FILE__, __LINE__);
		return;
	DebugPrint(__FILE__, __LINE__);}

	for(TSubclassOf<ABaseWeapon> weaponClass : DefaultWeaponClasses) {DebugPrint(__FILE__, __LINE__);
		FActorSpawnParameters spawnParams;
		spawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		spawnParams.Instigator = (APawn*) GetOwner();
		spawnParams.Owner = GetOwner();
		ABaseWeapon* spawnedWeapon = GetWorld()->SpawnActor<ABaseWeapon>(weaponClass, spawnParams);
		AddWeapon(spawnedWeapon);
	DebugPrint(__FILE__, __LINE__);}
DebugPrint(__FILE__, __LINE__);}

void UWeaponInventory::Destroy() {DebugPrint(__FILE__, __LINE__);
	for(ABaseWeapon* weapon : weapons) {DebugPrint(__FILE__, __LINE__);
		weapon->Destroy();
	DebugPrint(__FILE__, __LINE__);}
DebugPrint(__FILE__, __LINE__);}

// Called every frame
void UWeaponInventory::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) {DebugPrint(__FILE__, __LINE__);
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
DebugPrint(__FILE__, __LINE__);}

void UWeaponInventory::AddWeapon(ABaseWeapon* Weapon) {DebugPrint(__FILE__, __LINE__);
	weapons.AddUnique(Weapon);
DebugPrint(__FILE__, __LINE__);}

void UWeaponInventory::RemoveWeapon(ABaseWeapon* Weapon) {DebugPrint(__FILE__, __LINE__);
	weapons.Remove(Weapon);
DebugPrint(__FILE__, __LINE__);}

ABaseWeapon* UWeaponInventory::GetNextWeapon(ABaseWeapon* CurrentWeapon) {DebugPrint(__FILE__, __LINE__);
	int32 index = weapons.IndexOfByKey(CurrentWeapon);
	return weapons[(index + 1) % weapons.Num()];
DebugPrint(__FILE__, __LINE__);}

ABaseWeapon* UWeaponInventory::GetPreviousWeapon(ABaseWeapon* CurrentWeapon) {DebugPrint(__FILE__, __LINE__);
	int32 index = weapons.IndexOfByKey(CurrentWeapon);
	return weapons[(index - 1 + weapons.Num()) % weapons.Num()];
DebugPrint(__FILE__, __LINE__);}

ABaseWeapon* UWeaponInventory::GetWeapon(int index) {DebugPrint(__FILE__, __LINE__);
	if(index < 0 || index >= weapons.Num())
		return NULL;
	return weapons[index];
DebugPrint(__FILE__, __LINE__);}