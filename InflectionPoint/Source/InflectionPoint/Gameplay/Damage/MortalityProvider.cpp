// Fill out your copyright notice in the Description page of Project Settings.

#include "InflectionPoint.h"
#include "MortalityProvider.h"


UMortalityProvider::UMortalityProvider() {DebugPrint(__FILE__, __LINE__);

	PrimaryComponentTick.bCanEverTick = false;

DebugPrint(__FILE__, __LINE__);}

void UMortalityProvider::BeginPlay() {DebugPrint(__FILE__, __LINE__);
	Super::BeginPlay();
	CurrentHealth = StartHealth;
DebugPrint(__FILE__, __LINE__);}

void UMortalityProvider::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const {DebugPrint(__FILE__, __LINE__);
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UMortalityProvider, CurrentHealth);
DebugPrint(__FILE__, __LINE__);}

void UMortalityProvider::TakeDamage(float DamageAmount, AController * KillingPlayer, AActor * DamageCauser) {DebugPrint(__FILE__, __LINE__);
	if(CurrentHealth <= 0 || Invincible) {DebugPrint(__FILE__, __LINE__);
		return;
	DebugPrint(__FILE__, __LINE__);}

	int oldHealth = CurrentHealth;
	CurrentHealth -= DamageAmount;
	OnHealthChanged.Broadcast(oldHealth, CurrentHealth);

	if(CurrentHealth <= 0) {DebugPrint(__FILE__, __LINE__);
		OnDeath.Broadcast(KillingPlayer, DamageCauser);
	DebugPrint(__FILE__, __LINE__);}
DebugPrint(__FILE__, __LINE__);}

bool UMortalityProvider::IsAlive() {DebugPrint(__FILE__, __LINE__);
	return CurrentHealth > 0;
DebugPrint(__FILE__, __LINE__);}