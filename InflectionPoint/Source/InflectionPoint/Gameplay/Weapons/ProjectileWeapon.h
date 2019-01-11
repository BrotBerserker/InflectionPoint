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
	/* -------------- */
	/*     Config     */
	/* -------------- */

	/** Projectile class to spawn */
	UPROPERTY(EditDefaultsOnly, Category = Projectile)
		TSubclassOf<class AInflectionPointProjectile> ProjectileClass;

public:
	/* -------------- */
	/*   Components   */
	/* -------------- */

	/** Beam particle, starts at hand, ends at the current target */
	UPROPERTY(VisibleDefaultsOnly)
		UParticleSystemComponent* TargetBeam;

public:
	/** The currently selected target component */
	UPROPERTY(BlueprintReadWrite)
		UPrimitiveComponent* SelectedTargetComponent;

public:

	AProjectileWeapon();

	virtual void Tick(float DeltaTime) override;

	void SwitchSelectedTarget(UPrimitiveComponent * newTarget);

	UPrimitiveComponent* FindSelectedTarget();

	void MarkTarget(UPrimitiveComponent* targetComponent);

	void UnMarkTarget(UPrimitiveComponent* targetComponent);

	void UpdateTargetBeam();

	bool TargetShouldBeDeselected(UPrimitiveComponent* targetComponent);

	virtual bool CanFire() override;

	virtual void ExecuteFire() override;

	virtual void OnUnequip() override;

};
