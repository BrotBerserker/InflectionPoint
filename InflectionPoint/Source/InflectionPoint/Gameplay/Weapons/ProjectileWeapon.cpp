// Fill out your copyright notice in the Description page of Project Settings.

#include "InflectionPoint.h"
#include "Gameplay/Characters/BaseCharacter.h"
#include "ProjectileWeapon.h"

AProjectileWeapon::AProjectileWeapon() {DebugPrint(__FILE__, __LINE__);
	AISuitabilityWeaponRangeCurve.GetRichCurve()->AddKey(1000, 1.0);
}

void AProjectileWeapon::ExecuteFire() {DebugPrint(__FILE__, __LINE__);
	UWorld* const World = GetWorld();
	if(ProjectileClass == NULL || !AssertNotNull(World, GetWorld(), __FILE__, __LINE__))
		return;

	//Set Spawn Collision Handling Override
	FActorSpawnParameters ActorSpawnParams;
	ActorSpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	ActorSpawnParams.Instigator = Instigator;
	ActorSpawnParams.Owner = this;

	// spawn the projectile at the muzzle
	AInflectionPointProjectile* projectile = World->SpawnActor<AInflectionPointProjectile>(ProjectileClass, GetFPMuzzleLocation(), GetAimDirection(), ActorSpawnParams);
}