// Fill out your copyright notice in the Description page of Project Settings.

#include "InflectionPoint.h"
#include "MortalityProvider.h"


UMortalityProvider::UMortalityProvider() {DebugPrint(__FILE__, __LINE__);
	PrimaryComponentTick.bCanEverTick = true;
DebugPrint(__FILE__, __LINE__);}

void UMortalityProvider::BeginPlay() {DebugPrint(__FILE__, __LINE__);
	Super::BeginPlay();
	CurrentHealth = StartHealth;
	CurrentShield = StartShield;
DebugPrint(__FILE__, __LINE__);}

void UMortalityProvider::TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction *ThisTickFunction) {DebugPrint(__FILE__, __LINE__);
	if(!IsAlive())
		return;
	if(GetOwner() && !GetOwner()->HasAuthority())
		return;

	if(invincibleSeconds > 0) {DebugPrint(__FILE__, __LINE__);
		invincibleSeconds -= DeltaTime;
		Invincible = invincibleSeconds > 0;
	DebugPrint(__FILE__, __LINE__);}

	timeSinceLastDamageTaken += DeltaTime; 
	if(timeSinceLastDamageTaken < RegenerationDelayAfterDamageTaken)
		return;

	timeSinceLastRegeneration += DeltaTime;
	if(timeSinceLastRegeneration > ShieldRegenerationInterval) {DebugPrint(__FILE__, __LINE__);
		timeSinceLastRegeneration -= ShieldRegenerationInterval;
		CurrentShield = FMath::Min(CurrentShield + ShieldRegenerationAmount, StartShield);
	DebugPrint(__FILE__, __LINE__);}
DebugPrint(__FILE__, __LINE__);}

void UMortalityProvider::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const {DebugPrint(__FILE__, __LINE__);
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UMortalityProvider, CurrentHealth);
	DOREPLIFETIME(UMortalityProvider, CurrentShield);
DebugPrint(__FILE__, __LINE__);}

void UMortalityProvider::TakeDamage(float DamageAmount, AController * KillingPlayer, AActor * DamageCauser) {DebugPrint(__FILE__, __LINE__);
	if(CurrentHealth <= 0 || Invincible) 
		return;
	timeSinceLastDamageTaken = 0;

	if(CurrentShield > 0) {DebugPrint(__FILE__, __LINE__);
		if(CurrentShield >= DamageAmount) {DebugPrint(__FILE__, __LINE__);
			CurrentShield -= DamageAmount;
			DamageAmount = 0;
		DebugPrint(__FILE__, __LINE__);} else {DebugPrint(__FILE__, __LINE__);
			DamageAmount -= CurrentShield;
			CurrentShield = 0;
		DebugPrint(__FILE__, __LINE__);}
	DebugPrint(__FILE__, __LINE__);}

	int oldHealth = CurrentHealth;
	CurrentHealth -= DamageAmount;
	OnHealthChanged.Broadcast(oldHealth, CurrentHealth);

	if(CurrentHealth <= 0) {DebugPrint(__FILE__, __LINE__);
		OnDeath.Broadcast(KillingPlayer, DamageCauser);
	DebugPrint(__FILE__, __LINE__);}
DebugPrint(__FILE__, __LINE__);}

void UMortalityProvider::SetInvincibleForSeconds(float Seconds) {DebugPrint(__FILE__, __LINE__);
	invincibleSeconds = Seconds;
DebugPrint(__FILE__, __LINE__);}

bool UMortalityProvider::IsAlive() {DebugPrint(__FILE__, __LINE__);
	return CurrentHealth > 0;
DebugPrint(__FILE__, __LINE__);}