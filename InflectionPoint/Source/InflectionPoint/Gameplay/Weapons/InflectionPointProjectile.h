// Copyright 1998-2016 Epic Games, Inc. All Rights Reserved.
#pragma once
#include "GameFramework/Actor.h"
#include "InflectionPointProjectile.generated.h"

UCLASS(config = Game)
class AInflectionPointProjectile : public AActor {
	GENERATED_BODY()

public:
	/* ---------------------- */
	/*  Blueprint Properties  */
	/* ---------------------- */

	/** Sphere collision component */
	UPROPERTY(VisibleDefaultsOnly, Category = Projectile)
		class USphereComponent* CollisionComp;

	/** Projectile movement component */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"))
		class UProjectileMovementComponent* ProjectileMovement;

public:
	/* ------------- */
	/*   Functions   */
	/* ------------- */

	/** Constructor, sets up collision and movement */
	AInflectionPointProjectile();

	/** BeginPlay, adds instigator to the collisionComp's ignored actors */
	virtual void BeginPlay() override;

	/** called when projectile hits something */
	UFUNCTION()
		void OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);
};

