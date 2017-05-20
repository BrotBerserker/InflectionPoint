// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Components/ActorComponent.h"
#include "CollisionDamageDealer.generated.h"


UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class INFLECTIONPOINT_API UCollisionDamageDealer : public UActorComponent {
	GENERATED_BODY()
		DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnDamageHitDelegate, float, damage, FVector, NormalImpulse, const FHitResult&, Hit);
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnHarmlessHitDelegate, FVector, NormalImpulse, const FHitResult&, Hit);
public:
	// Sets default values for this component's properties
	UCollisionDamageDealer();

public:

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "InflectionPoint|Damage")
		bool DealDamageOnlyOnCharacters = true;
	/* Destroys the projectile when it collides with an actor that shouldn't be damaged by this projectile */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "InflectionPoint|Damage")
		bool DestroyOnHarmlessHit = false;
	/* Destroys the projectile when it collides with an actor that should be damaged by this projectile (even if the damage is 0) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "InflectionPoint|Damage")
		bool DestroyOnDamageDealt = false;
	/* Lifetime after Hit when DestroyOnHit or DestroyOnActorHit enabled */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "InflectionPoint|Damage")
		float DestroyDelay = 0.;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "InflectionPoint|Damage")
		TSubclassOf<UDamageType> DamageType;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "InflectionPoint|Damage")
		float Damage = 0.;

	UFUNCTION()
		void OnHit(class UPrimitiveComponent* HitComp, class AActor* OtherActor, class UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);

	UPROPERTY(BlueprintAssignable)
		FOnDamageHitDelegate OnDamageHit;

	UPROPERTY(BlueprintAssignable)
		FOnHarmlessHitDelegate OnHarmlessHit;

	float InflictDamage(AActor* DamagedActor);

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

private:
	UShapeComponent* CollisionShapeComponent;

	void PerformHitConsequences(bool damageDealed);

	void SpawnFromClassOnHit(const FHitResult & Hit, UClass * &item);
	void DestroyOwner();
};
