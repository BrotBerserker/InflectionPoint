// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Components/ActorComponent.h"
#include "MortalityProvider.generated.h"


UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class INFLECTIONPOINT_API UMortalityProvider : public UActorComponent {
	GENERATED_BODY()

		DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnHealthChangedDelegate, int, oldHealth, int, newHealth);
	DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnDeathDelegate);

public:
	UMortalityProvider();

	virtual void BeginPlay() override;

	void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	/* The health to start with */
	UPROPERTY(BlueprintReadOnly, EditAnywhere, meta = (ClampMin = 0))
		int StartHealth = 100;

	/* When health reaches 0, this is the time the MortalityProvider will wait before destroying its owner. */
	UPROPERTY(EditAnywhere)
		float SecondsToLiveBeforeDestruction = 0.f;

public:

	/* Inflicts damage by reducing the health value */
	UFUNCTION(BlueprintCallable, category = "InflectionPoint|Damage")
		void TakeDamage(float DamageAmount);

	/* Fired when the current health value changes */
	UPROPERTY(BlueprintAssignable)
		FOnHealthChangedDelegate OnHealthChanged;

	/* Fired when the current health value reaches 0. The timer to destroy the owning actor has already been started at this point. */
	UPROPERTY(BlueprintAssignable)
		FOnDeathDelegate OnDeath;

	void Die();

private:
	UPROPERTY(Replicated)
		int CurrentHealth;

public:
	UFUNCTION(BlueprintCallable, category = "InflectionPoint|Damage")
		FORCEINLINE int GetCurrentHealth() { return CurrentHealth; }

};
