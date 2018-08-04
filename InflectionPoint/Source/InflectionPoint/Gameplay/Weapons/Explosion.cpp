// Fill out your copyright notice in the Description page of Project Settings.

#include "InflectionPoint.h"
#include "Gameplay/Damage/DamageTypes/DefaultDamageType.h"
#include "Explosion.h"


// Sets default values
AExplosion::AExplosion() {DebugPrint(__FILE__, __LINE__);
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	RadialDamageDealer = CreateDefaultSubobject<URadialDamageDealer>(TEXT("RadialDamageDealer"));
	RadialDamageDealer->DealDamageOnBeginPlay = false;
	RadialDamageDealer->DamageTypeClass = UDefaultDamageType::StaticClass();
}

// Called when the game starts or when spawned
void AExplosion::BeginPlay() {DebugPrint(__FILE__, __LINE__);
	Super::BeginPlay();
	// Set the Weapon as DamageDealer
	RadialDamageDealer->DamageCauser = GetOwner();
	RadialDamageDealer->DealDamage();

	if(LifeSpan > 0)
		SetLifeSpan(LifeSpan);
}

