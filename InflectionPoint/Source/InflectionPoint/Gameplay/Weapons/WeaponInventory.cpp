// Fill out your copyright notice in the Description page of Project Settings.

#include "InflectionPoint.h"
#include "WeaponInventory.h"


// Sets default values for this component's properties
UWeaponInventory::UWeaponInventory() {DebugPrint(__FILE__, __LINE__);
	PrimaryComponentTick.bCanEverTick = false;
}

// Called when the game starts
void UWeaponInventory::BeginPlay() {DebugPrint(__FILE__, __LINE__);
	Super::BeginPlay();

	if(!GetOwner()->HasAuthority()) {DebugPrint(__FILE__, __LINE__);
		return;
	}

	for(TSubclassOf<ABaseWeapon> weaponClass : DefaultWeaponClasses) {DebugPrint(__FILE__, __LINE__);
		FActorSpawnParameters spawnParams;
		spawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		spawnParams.Instigator = (APawn*) GetOwner();
		spawnParams.Owner = GetOwner();
		ABaseWeapon* spawnedWeapon = GetWorld()->SpawnActor<ABaseWeapon>(weaponClass, spawnParams);
		AddWeapon(spawnedWeapon);
	}
}

void UWeaponInventory::Destroy() {DebugPrint(__FILE__, __LINE__);
	for(ABaseWeapon* weapon : weapons) {DebugPrint(__FILE__, __LINE__);
		weapon->Destroy();
	}
}

// Called every frame
void UWeaponInventory::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) {DebugPrint(__FILE__, __LINE__);
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

void UWeaponInventory::AddWeapon(ABaseWeapon* Weapon) {DebugPrint(__FILE__, __LINE__);
	weapons.AddUnique(Weapon);
}

void UWeaponInventory::RemoveWeapon(ABaseWeapon* Weapon) {DebugPrint(__FILE__, __LINE__);
	weapons.Remove(Weapon);
}

ABaseWeapon* UWeaponInventory::GetNextWeapon(ABaseWeapon* CurrentWeapon) {DebugPrint(__FILE__, __LINE__);
	int32 index = weapons.IndexOfByKey(CurrentWeapon);
	return weapons[(index + 1) % weapons.Num()];
}

ABaseWeapon* UWeaponInventory::GetPreviousWeapon(ABaseWeapon* CurrentWeapon) {DebugPrint(__FILE__, __LINE__);
	int32 index = weapons.IndexOfByKey(CurrentWeapon);
	return weapons[(index - 1 + weapons.Num()) % weapons.Num()];
}

ABaseWeapon* UWeaponInventory::GetWeapon(int index) {DebugPrint(__FILE__, __LINE__);
	if(index < 0 || index >= weapons.Num())
		return NULL;
	return weapons[index];
}

int UWeaponInventory::GetWeaponNum() {DebugPrint(__FILE__, __LINE__);
	return weapons.Num();
}
