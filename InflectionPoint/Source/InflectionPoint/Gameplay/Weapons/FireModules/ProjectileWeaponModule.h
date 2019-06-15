// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Gameplay/Weapons/FireModules/BaseWeaponModule.h"
#include "Gameplay/Characters/BaseCharacter.h"
#include "ProjectileWeaponModule.generated.h"

/**
 * 
 */
UCLASS(BlueprintType, Blueprintable)
class INFLECTIONPOINT_API UProjectileWeaponModule : public UBaseWeaponModule
{
	GENERATED_BODY()

public:
	/* -------------- */
	/*     Config     */
	/* -------------- */

	/** Projectile class to spawn */
	UPROPERTY(EditDefaultsOnly, Category = Projectile)
		TSubclassOf<class AInflectionPointProjectile> ProjectileClass;

	UPROPERTY(EditDefaultsOnly, Category = Projectile)
		float TargetSelectingRange = 30000.0f;
	
	/** Beam particle, starts at hand, ends at the current target */
	UPROPERTY(EditDefaultsOnly, Category = Effects)
		UParticleSystem* TargetBeam;


public:
	/** The currently selected target component */
	UPROPERTY(BlueprintReadWrite)
		UPrimitiveComponent* SelectedTargetComponent;

	UPROPERTY()
		UParticleSystemComponent* TargetBeamComponent;

public:

	virtual void Tick(float DeltaTime) override;

	void SwitchSelectedTarget(UPrimitiveComponent * newTarget);

	UPrimitiveComponent* FindSelectedTarget();

	void MarkTarget(UPrimitiveComponent* targetComponent);

	void UnMarkTarget(UPrimitiveComponent* targetComponent);

	void UpdateTargetBeam();

	bool TargetShouldBeDeselected(UPrimitiveComponent* targetComponent);
	
	virtual void Initialize() override;
	virtual bool CanFire() override;
	virtual void ExecuteFire() override;
	virtual void OnActivate() override;
	virtual void OnDeactivate() override;
};
