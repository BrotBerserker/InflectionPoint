// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Gameplay/Weapons/FireModules/BaseWeaponModule.h"
#include "Gameplay/Characters/BaseCharacter.h"
#include "Gameplay/Weapons/BaseWeapon.h"
#include "Gameplay/BuildableActor.h"
#include "BuildingWeaponModule.generated.h"

/**
 * 
 */
UCLASS()
class INFLECTIONPOINT_API UBuildingWeaponModule : public UBaseWeaponModule
{
	GENERATED_BODY()
public:
	//UPROPERTY(VisibleDefaultsOnly)
	//	UTextRenderComponent* OutOfRangeText;

public:

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly)
		TSubclassOf<ABuildableActor> BuildableActorClass;

	UPROPERTY(EditDefaultsOnly, Category = Projectile)
		float BuildingRange = 500.f;

public:
	UFUNCTION(NetMulticast, Reliable)
		void MulticastHidePreview();

public:
	//ABuildingWeapon();

	virtual void Tick(float DeltaTime) override;

	virtual bool CanFire() override;

	virtual void ExecuteFire() override;

	virtual void OnDeactivate() override;

private:
	ABuildableActor* buildableActor;

	void UpdateBuildableActor(FHitResult &HitResult);

	void ShowOutOfRange();
};
