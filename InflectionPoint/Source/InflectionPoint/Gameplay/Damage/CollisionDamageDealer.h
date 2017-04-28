// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Components/ActorComponent.h"
#include "CollisionDamageDealer.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class INFLECTIONPOINT_API UCollisionDamageDealer : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UCollisionDamageDealer();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	/* Dont forget to set this*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage")
		AController * DamageEventInstigator;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage")
		bool DealDamageOnlyOnCharacters = true;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage")
		bool DestroyOnHitOnly = false;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage")
		bool DestroyOnDamageDealed = false;
	/* Lifetime after Hit when DestroyOnHit or DestroyOnActorHit enabled*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage")
		float DestroyDelay = 0.;
	/* Dont forget to set this*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage")
		TSubclassOf<UDamageType> DamageType;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage")
		float Damage = 0.;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage")
		TArray<UClass*> ClassesToSpawnOnDamageDealed;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage")
		TArray<UClass*> ClassesToSpawnOnHitOnly;

	UShapeComponent* CollisionShapeComponent;

	UFUNCTION()
		void OnHit(class UPrimitiveComponent* HitComp, class AActor* OtherActor, class UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);
	
	bool InflictDamage(AActor* DamagedActor);

private:
	void PerformHitConsequences(bool damageDealed, const FHitResult & Hit);

	void SpawnFromClassOnHit(const FHitResult & Hit, UClass * &item);
	void DestroyOwner();
};
