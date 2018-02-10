// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Gameplay/Weapons/InflectionPointProjectile.h"
#include "BaseWeapon.generated.h"

class ABaseCharacter;

UCLASS()
class INFLECTIONPOINT_API ABaseWeapon : public AActor {
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	ABaseWeapon();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	/** Gun mesh: 1st person view (seen only by self) */
	UPROPERTY(VisibleDefaultsOnly, Category = Mesh)
		class USkeletalMeshComponent* Mesh1P;

	/** Gun mesh: 3rd person view (seen only by others) */
	UPROPERTY(VisibleDefaultsOnly, Category = Mesh)
		class USkeletalMeshComponent* Mesh3P;

	/** Location on gun mesh where projectiles should spawn. */
	UPROPERTY(VisibleDefaultsOnly, Category = Mesh)
		class USceneComponent* FP_MuzzleLocation;

	/** Projectile class to spawn */
	UPROPERTY(EditDefaultsOnly, Category = Projectile)
		TSubclassOf<class AInflectionPointProjectile> ProjectileClass;

	/** Notifies Clients about projectile fired */
	UFUNCTION(Unreliable, NetMulticast)
		void MulticastProjectileFired();

	/** Sound to play each time we fire */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Projectile)
		class USoundBase* FireSound;

	/** AnimMontage to play each time we fire */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Projectile)
		class UAnimMontage* FireAnimation;

	/** Returns the location at which a projectile should spawn */
	FVector GetProjectileSpawnLocation();

	/** Returns the rotation with which a projectile should spawn */
	FRotator GetProjectileSpawnRotation();

	ABaseCharacter* OwningCharacter;

	UPROPERTY(Replicated, BlueprintReadWrite)
		int CurrentAmmo;

	/** Number of shots per clip */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = Projectile)
		int MaxAmmo = 7;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	virtual void StartFire();

	virtual void StopFire();

	virtual void Reload();
};


