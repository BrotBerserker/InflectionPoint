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
	/* -------------- */
	/*   Components   */
	/* -------------- */

	/** Gun mesh: 1st person view (seen only by self) */
	UPROPERTY(VisibleDefaultsOnly, Category = Mesh)
		class USkeletalMeshComponent* Mesh1P;

	/** Gun mesh: 3rd person view (seen only by others) */
	UPROPERTY(VisibleDefaultsOnly, Category = Mesh)
		class USkeletalMeshComponent* Mesh3P;

	/** Location on Mesh1P where projectiles and effects should spawn. */
	UPROPERTY(VisibleDefaultsOnly, Category = Mesh)
		class USceneComponent* FP_MuzzleLocation;

	/** Location on Mesh3P where effects should spawn. */
	UPROPERTY(VisibleDefaultsOnly, Category = Mesh)
		class USceneComponent* TP_MuzzleLocation;

public:
	/* ---------------------- */
	/*    Editor Settings     */
	/* ---------------------- */

	/** Sound to play each time we fire */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Sound)
		class USoundBase* FireSound;

	/** AnimMontage to play each time we fire */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Animation)
		class UAnimMontage* FireAnimation;

	/** AnimMontage to play on 1P hands when reloading */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Animation)
		class UAnimMontage* ReloadAnimation1P;

	/** AnimMontage to play on 3p hands when reloading */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Animation)
		class UAnimMontage* ReloadAnimation3P;

	/** AnimMontage to play on 1p weapon when reloading */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Animation)
		class UAnimationAsset* ReloadAnimationWeapon1P;

	/** FX for muzzle flash */
	UPROPERTY(EditDefaultsOnly, Category = Effects)
		UParticleSystem* MuzzleFX;

	/** Duration for the muzzle FX flash (-1 for endless)*/
	UPROPERTY(EditDefaultsOnly, Category = Effects)
		float MuzzleFXDuration = 0.1;

	/** AnimMontage to play on Mesh1P when this weapon is equipped */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Animation)
		class UAnimMontage* EquipAnimation1P;

	/** AnimMontage to playon Mesh3P when this weapon is equipped */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Animation)
		class UAnimMontage* EquipAnimation3P;

	/** Number of shots per clip */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = WeaponConfig)
		int MaxAmmo = 7;

	/** Whether automatic fire should be enabled for this weapon */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = WeaponConfig)
		bool AutoFire = true;

	/** Seconds to wait between two shots */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = WeaponConfig)
		float FireInterval = 1.0f;

public:
	/* ------------- */
	/*   Functions   */
	/* ------------- */

	/** Constructor, initializes components */
	ABaseWeapon();

	/** Initializes variables and attachments */
	virtual void BeginPlay() override;

	/** Executes behaviour depending on the current state */
	virtual void Tick(float DeltaTime) override;

	/** If possible, changes the current state to FIRING */
	virtual void StartFire();

	/** If currently firing, changes the current state to IDLE */
	virtual void StopFire();

	/** Fires a shot (includes animation, sound, and decreasing ammo) */
	virtual void Fire();

	/** This function should be overriden in subclasses to implement specific fire behaviour */
	virtual void ExecuteFire() PURE_VIRTUAL(ABaseWeapon::ExecuteFire, ;);

	/** Plays the reload animation (only if the weapon doesn't have max ammo) */
	UFUNCTION()
		virtual void Reload();

	/** Plays the reload animation on all clients */
	UFUNCTION(NetMulticast, Reliable)
		virtual void MulticastPlayReloadAnimation();

	/** Called when this weapon is equipped. Sets up attachment, plays equip animation etc. */
	virtual void OnEquip();

	/** Called when this weapon is unequipped. Removes attachment, stops animations etc. */
	virtual void OnUnequip();

	/** Attaches this weapon's meshes to the owning character */
	void AttachToOwner();

	/** Dettaches this weapon's meshes from the owning character */
	void DetachFromOwner();

	/* Spawns the Fire Sound (called from multicast)*/
	void SpawnFireSound();

	/* Plays the Fire Animation (called from multicast)*/
	void PlayFireAnimation();

	/* Spawns the MuzzleFX for 1P and 3P (called from multicast)*/
	void SpawnMuzzleFX();

	/** Callback for anim notifies during the reload animation */
	UFUNCTION()
		void ReloadAnimationNotifyCallback(FName NotifyName, const FBranchingPointNotifyPayload& BranchingPointPayload);

	/** Callback for when the reload animation ends or is interrupted */
	UFUNCTION()
		void ReloadAnimationEndCallback(UAnimMontage* Montage, bool bInterrupted);

	/** Returns the 1st person muzzle location */
	FVector GetFPMuzzleLocation();
	UFUNCTION()
		void DecativateParticleSystem(UParticleSystemComponent* effect);

	/** Returns the 3rd person muzzle location */
	FVector GetTPMuzzleLocation();

	/** Returns the owning character's aim direction */
	FRotator GetAimDirection();

	/** Notifies clients about projectile fired (plays animation, sound etc.) */
	UFUNCTION(Reliable, NetMulticast)
		void MulticastFireExecuted();

public:
	UPROPERTY(BlueprintReadWrite, Replicated)
		ABaseCharacter* OwningCharacter;

	UPROPERTY(Replicated, BlueprintReadWrite)
		int CurrentAmmo;

protected:
	EWeaponState CurrentState = EWeaponState::IDLE;

	UPlayerStateRecorder* Recorder;

	float LastShotTimeStamp = 0.f;
	float passedTime = 0.f;

	FScriptDelegate AnimationNotifyDelegate;
	FScriptDelegate AnimationEndDelegate;

	bool equipped = false;
	bool IsReplaySimulatedFirePressed = false;
};