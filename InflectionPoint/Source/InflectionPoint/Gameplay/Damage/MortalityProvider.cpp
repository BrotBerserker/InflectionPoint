// Fill out your copyright notice in the Description page of Project Settings.

#include "InflectionPoint.h"
#include "MortalityProvider.h"


UMortalityProvider::UMortalityProvider() {
	PrimaryComponentTick.bCanEverTick = true;
}

void UMortalityProvider::BeginPlay() {
	Super::BeginPlay();
	CurrentHealth = StartHealth;
	CurrentShield = StartShield;
}

void UMortalityProvider::TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction *ThisTickFunction) {
	if(!IsAlive())
		return;
	if(GetOwner() && !GetOwner()->HasAuthority())
		return;

	timeSinceLastDamageTaken += DeltaTime; 
	if(timeSinceLastDamageTaken < RegenerationDelayAfterDamageTaken)
		return;

	timeSinceLastRegeneration += DeltaTime;
	if(timeSinceLastRegeneration > ShieldRegenerationInterval) {
		timeSinceLastRegeneration -= ShieldRegenerationInterval;
		CurrentShield = FMath::Min(CurrentShield + ShieldRegenerationAmount, StartShield);
	}
}

void UMortalityProvider::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const {
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UMortalityProvider, CurrentHealth);
	DOREPLIFETIME(UMortalityProvider, CurrentShield);
}

void UMortalityProvider::TakeDamage(float DamageAmount, AController * KillingPlayer, AActor * DamageCauser) {
	if(CurrentHealth <= 0 || Invincible) 
		return;
	timeSinceLastDamageTaken = 0;

	if(CurrentShield > 0) {
		if(CurrentShield >= DamageAmount) {
			CurrentShield -= DamageAmount;
			DamageAmount = 0;
		} else {
			DamageAmount -= CurrentShield;
			CurrentShield = 0;
		}
	}

	int oldHealth = CurrentHealth;
	CurrentHealth -= DamageAmount;
	OnHealthChanged.Broadcast(oldHealth, CurrentHealth);

	if(CurrentHealth <= 0) {
		OnDeath.Broadcast(KillingPlayer, DamageCauser);
	}
}

bool UMortalityProvider::IsAlive() {
	return CurrentHealth > 0;
}