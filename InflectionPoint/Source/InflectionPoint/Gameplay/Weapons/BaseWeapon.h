// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Gameplay/Weapons/InflectionPointProjectile.h"
#include "BaseWeapon.generated.h"

class ABaseCharacter;
class UPlayerStateRecorder;

UENUM(BlueprintType)
enum EWeaponState {
	IDLE,
	RELOADING,
	FIRING
};
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

	/** Location on gun mesh where projectiles should spawn. */
	UPROPERTY(VisibleDefaultsOnly, Category = Mesh)
		class USceneComponent* TP_MuzzleLocation;

	/** Notifies Clients about projectile fired */
	UFUNCTION(Reliable, NetMulticast)
		void MulticastProjectileFired();

	/** Sound to play each time we fire */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Sound)
		class USoundBase* FireSound;

	/** AnimMontage to play each time we fire */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Animation)
		class UAnimMontage* FireAnimation;

	/** AnimMontage to play when reloading */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Animation)
		class UAnimMontage* ReloadAnimation;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Animation)
		class UAnimMontage* EquipAnimation;

	UPROPERTY(BlueprintReadWrite, Replicated)
		ABaseCharacter* OwningCharacter;

	UPlayerStateRecorder* Recorder;

	UPROPERTY(Replicated, BlueprintReadWrite)
		int CurrentAmmo;

	/** Number of shots per clip */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = WeaponConfig)
		int MaxAmmo = 7;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = WeaponConfig)
		bool AutoFire = true;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = WeaponConfig)
		float FireInterval = 1.0f;

	EWeaponState CurrentState = EWeaponState::IDLE;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	virtual void StartFire();

	virtual void Fire();

	/* Override this Function */
	virtual void ExecuteFire() PURE_VIRTUAL(ABaseWeapon::ExecuteFire, ;);

	virtual void StopFire();

	UFUNCTION()
		virtual void Reload();

	virtual void OnEquip();

	virtual void OnUnequip();

	void AttachToOwner();

	void DetachFromOwner();

	UFUNCTION(NetMulticast, Reliable)
		virtual void MulticastPlayReloadAnimation();

	UFUNCTION()
		void ReloadAnimationNotifyCallback(FName NotifyName, const FBranchingPointNotifyPayload& BranchingPointPayload);

	UFUNCTION()
		void ReloadAnimationEndCallback(UAnimMontage* Montage, bool bInterrupted);


	/** Returns the 1 Person muzzle location */
	FVector GetFPMuzzleLocation();
	/** Returns the 3 Person muzzle location */
	FVector GetTPMuzzleLocation();

	/** Returns the rotation where the player is aiming */
	FRotator GetAimDirection();

private:
	float LastShotTimeStamp = 0.f;
	float passedTime = 0.f;

	FScriptDelegate AnimationNotifyDelegate;
	FScriptDelegate AnimationEndDelegate;

	bool equipped = false;
};