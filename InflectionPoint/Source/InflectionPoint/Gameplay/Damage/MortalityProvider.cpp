// Fill out your copyright notice in the Description page of Project Settings.

#include "InflectionPoint.h"
#include "MortalityProvider.h"


UMortalityProvider::UMortalityProvider() {

	PrimaryComponentTick.bCanEverTick = true; // ^^

}

void UMortalityProvider::BeginPlay() {
	Super::BeginPlay();
	CurrentHealth = StartHealth;
}

void UMortalityProvider::TakeDamage(float DamageAmount, FDamageEvent const & DamageEvent, AController * EventInstigator, AActor * DamageCauser) {
	if(CurrentHealth <= 0) {
		return;
	}

	int oldHealth = CurrentHealth;
	CurrentHealth -= DamageAmount;
	OnHealthChanged.Broadcast(oldHealth, CurrentHealth);

	if(CurrentHealth <= 0) {
		Die();
	}
}

void UMortalityProvider::Die() {
	GetOwner()->SetLifeSpan(SecondsToLiveBeforeDestruction);
	OnDeath.Broadcast();
}
