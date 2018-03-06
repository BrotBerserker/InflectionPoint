// Copyright 1998-2016 Epic Games, Inc. All Rights Reserved.
#pragma once
#include "GameFramework/Actor.h"
#include "Gameplay/Damage/CollisionDamageDealer.h"
#include "DebugTools/DebugLineDrawer.h"
#include "InflectionPointProjectile.generated.h"

UCLASS(config = Game)
class AInflectionPointProjectile : public AActor {
	GENERATED_BODY()

public:
	/* ---------------- */
	/*    Components    */
	/* ---------------- */
	UPROPERTY(EditDefaultsOnly)
		UCollisionDamageDealer* CollisionDamageDealer;

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

	UPROPERTY(EditDefaultsOnly, Category = Projectile)
		TSubclassOf<AActor> HitEffectClass;

	UPROPERTY(EditDefaultsOnly)
		UDebugLineDrawer* DebugLineDrawer;

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

	UFUNCTION()
		void OnDamageHit(float Damage, const FHitResult& Hit);

	UFUNCTION()
		void OnHarmlessHit(const FHitResult& Hit);

	UFUNCTION(NetMulticast, Reliable, BlueprintCallable)
		void MulticastSpawnHitEffect();

private:
	FVector startPos;
	bool firstHit = true;
};

