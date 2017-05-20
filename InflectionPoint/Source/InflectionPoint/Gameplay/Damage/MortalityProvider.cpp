// Fill out your copyright notice in the Description page of Project Settings.

#include "InflectionPoint.h"
#include "MortalityProvider.h"


UMortalityProvider::UMortalityProvider() {

	PrimaryComponentTick.bCanEverTick = false; 

}

void UMortalityProvider::BeginPlay() {
	Super::BeginPlay();
	CurrentHealth = StartHealth;
}

void UMortalityProvider::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const {
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UMortalityProvider, CurrentHealth);
}

void UMortalityProvider::TakeDamage(float DamageAmount) {
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
	GetOwner()->SetLifeSpan(SecondsToLiveBeforeDestruction + 0.0000001); // 0 dose not destroy o0
	OnDeath.Broadcast();
}
