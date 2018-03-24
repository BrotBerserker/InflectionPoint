// Fill out your copyright notice in the Description page of Project Settings.

#include "InflectionPoint.h"
#include "Explosion.h"


// Sets default values
AExplosion::AExplosion() {
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	RadialDamageDealer = CreateDefaultSubobject<URadialDamageDealer>(TEXT("RadialDamageDealer"));
	RadialDamageDealer->DealDamageOnBeginPlay = false;

	// Set the Weapon as DamageDealer
	RadialDamageDealer->DamageCauser = GetOwner();
}

// Called when the game starts or when spawned
void AExplosion::BeginPlay() {
	Super::BeginPlay();
	RadialDamageDealer->DealDamage();
}

