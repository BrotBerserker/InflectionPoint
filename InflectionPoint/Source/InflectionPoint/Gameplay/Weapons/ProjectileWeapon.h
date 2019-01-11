// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Gameplay/Weapons/BaseWeapon.h"
#include "ProjectileWeapon.generated.h"

/**
 *
 */
UCLASS()
class INFLECTIONPOINT_API AProjectileWeapon : public ABaseWeapon {
	GENERATED_BODY()

public:

	/** Projectile class to spawn */
	UPROPERTY(EditDefaultsOnly, Category = Projectile)
		TSubclassOf<class AInflectionPointProjectile> ProjectileClass;

	UPROPERTY(VisibleDefaultsOnly)
		UParticleSystemComponent* TargetBeam;

public:

	AProjectileWeapon();

	virtual void Tick(float DeltaTime) override;

	void UpdateSelectedTarget();

	UFUNCTION(Server, WithValidation, Reliable)
		void ServerSetSelectedTarget(UPrimitiveComponent* NewTarget);

	void SetTargetMarkerVisibility(AActor* actor, bool visible);

	void ExecuteFire() override;

	virtual void OnUnequip() override;

};
