// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Components/ActorComponent.h"
#include "CollisionDamageDealer.generated.h"


UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class INFLECTIONPOINT_API UCollisionDamageDealer : public UActorComponent {
	GENERATED_BODY()

		DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnDamageHitDelegate, float, damage, const FHitResult&, Hit);
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnHarmlessHitDelegate, const FHitResult&, Hit);

public:
	/* ---------------------- */
	/*  Editor Settings   */
	/* ---------------------- */

	/** Destroys the projectile when it collides with an actor that shouldn't be damaged by this projectile */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "InflectionPoint|Damage")
		bool DestroyOnHarmlessHit = false;

	/** Destroys the projectile when it collides with an actor that should be damaged by this projectile */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "InflectionPoint|Damage")
		bool DestroyOnDamageDealt = false;

	/** Lifetime after Hit when DestroyOnHit or DestroyOnActorHit enabled */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "InflectionPoint|Damage")
		float DestroyDelay = 0.;

	/** Type of the damage that is dealt */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "InflectionPoint|Damage")
		TSubclassOf<UDamageType> DamageType;

	/** Default is the Owner */
	UPROPERTY(BlueprintReadWrite, Category = "InflectionPoint|Damage")
		AActor* DamageCauser;

	/** The amount of damage */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "InflectionPoint|Damage")
		float Damage = 0.;

public:
	/* ------------- */
	/*   Functions   */
	/* ------------- */

	/** Constructor, emtpy */
	UCollisionDamageDealer();

	/** BeginPlay, registers OnHit Event */
	virtual void BeginPlay() override;

	/** Called when overlapping with another actor. Deals damage and calls OnDamageHit. */
	UFUNCTION()
		void OnOverlap(class UPrimitiveComponent* HitComp, class AActor* OtherActor, class UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult & SweepResult);

	/** Called when colliding with another actor. Deals NO damage and calls OnHarmlessHit. */
	UFUNCTION()
		void OnCollision(class UPrimitiveComponent* HitComp, class AActor* OtherActor, class UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);

	/** Fired when this projectile hit another actor and dealt damage to it */
	UPROPERTY(BlueprintAssignable)
		FOnDamageHitDelegate OnDamageHit;

	/** Fired when this projectile hit another actor without dealing damage */
	UPROPERTY(BlueprintAssignable)
		FOnHarmlessHitDelegate OnHarmlessHit;

	/** Inflicts damage to the given actor */
	float InflictDamage(AActor* DamagedActor);


private:
	UShapeComponent* collisionShapeComponent;

	void PerformHitConsequences(bool damageDealed);

	void SpawnFromClassOnHit(const FHitResult & Hit, UClass * &item);

	void DestroyOwner();
};
