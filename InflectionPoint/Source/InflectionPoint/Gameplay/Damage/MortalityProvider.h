// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Components/ActorComponent.h"
#include "MortalityProvider.generated.h"


UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class INFLECTIONPOINT_API UMortalityProvider : public UActorComponent {
	GENERATED_BODY()

		DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnHealthChangedDelegate, int, oldHealth, int, newHealth);
		DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnDeathDelegate, AController*, KillingPlayer, AActor*, DamageCauser);

public:
	/* ---------------------- */
	/*   Editor Settings      */
	/* ---------------------- */

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		bool Invincible = false;

	/* The health to start with */
	UPROPERTY(BlueprintReadOnly, EditAnywhere, meta = (ClampMin = 0))
		int StartHealth = 100;

	/* The shield to start with */
	UPROPERTY(BlueprintReadOnly, EditAnywhere, meta = (ClampMin = 0))
		int StartShield = 0;

	/* Length of regeneration intervals */
	UPROPERTY(BlueprintReadOnly, EditAnywhere, meta = (ClampMin = 0))
		float ShieldRegenerationInterval = 0.1f;

	/* How much shield gets regenerated in each interval */
	UPROPERTY(BlueprintReadOnly, EditAnywhere, meta = (ClampMin = 0))
		int ShieldRegenerationAmount = 1;

	/* How much shield gets regenerated in each interval */
	UPROPERTY(BlueprintReadOnly, EditAnywhere, meta = (ClampMin = 0))
		float RegenerationDelayAfterDamageTaken = 3.f;

	/* When health reaches 0, this is the time the MortalityProvider will wait before destroying its owner. */
	UPROPERTY(EditAnywhere)
		float SecondsToLiveBeforeDestruction = 0.f;

public:
	/* ---------------------- */
	/*		Properties		  */
	/* ---------------------- */

	UPROPERTY(Replicated, BlueprintReadOnly)
		int CurrentHealth;

	UPROPERTY(Replicated, BlueprintReadOnly)
		int CurrentShield;

public:
	/* ------------- */
	/*   Functions   */
	/* ------------- */

	/** Constructor */
	UMortalityProvider();

	/** BeginPlay, initializes health */
	virtual void BeginPlay() override;
	
	virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction *ThisTickFunction) override;

	/** Needed for replication of currentHealth */
	void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	/* Inflicts damage by reducing the health value */
	UFUNCTION(BlueprintCallable, category = "InflectionPoint|Damage")
		void TakeDamage(float DamageAmount, AController * KillingPlayer, AActor * DamageCauser);

	/* Fired when the current health value changes */
	UPROPERTY(BlueprintAssignable)
		FOnHealthChangedDelegate OnHealthChanged;

	/* Fired when the current health value reaches 0. */
	UPROPERTY(BlueprintAssignable)
		FOnDeathDelegate OnDeath;

	UFUNCTION(BlueprintCallable)
		bool IsAlive();
private:
	float timeSinceLastRegeneration;
	float timeSinceLastDamageTaken;
};
