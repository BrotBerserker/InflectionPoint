// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Gameplay/Damage/RadialDamageDealer.h"
#include "Explosion.generated.h"

UCLASS()
class INFLECTIONPOINT_API AExplosion : public AActor {
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	AExplosion();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:

	/* ---------------- */
	UPROPERTY(EditDefaultsOnly)
		URadialDamageDealer* RadialDamageDealer;
public:

	/* -1 for infinit lifespan*/
	UPROPERTY(EditDefaultsOnly)
		float LifeSpan = 10;
};
