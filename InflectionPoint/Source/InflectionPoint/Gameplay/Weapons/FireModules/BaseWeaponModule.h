// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "UnrealNetwork.h"
#include "Gameplay/Recording/PlayerStateRecorder.h"
#include "BaseWeaponModule.generated.h"

UENUM(Blueprintable)
enum class EFireMode : uint8 {
	Primary = 0,
	Secondary = 1,
};

UENUM(BlueprintType)
enum class EWeaponModuleState : uint8 {
	DEACTIVATED,
	IDLE,
	CHARGING,
	FIRING
};

/**
 *
 */
UCLASS(BlueprintType, Blueprintable)
class INFLECTIONPOINT_API UBaseWeaponModule : public UActorComponent {
	GENERATED_BODY()
public:
	// ===
	// Hint: RPC calls are not working for replicated UObjects
	// ===
	// https://wiki.unrealengine.com/Replication#Advanced:_Generic_replication_of_Actor_Subobjects
	/** Enables replication for UObject */
	//virtual bool IsSupportedForNetworking() const override { return true; }	
	UBaseWeaponModule();
public:
	/* ---------------------- */
	/*    Editor Settings     */
	/* ---------------------- */

	/** Sound to play when no Ammo is left */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Sound)
		class USoundBase* NoAmmoSound;

	/** Sound to play each time we fire */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Sound)
		class USoundBase* ChargeSound;

	/** Sound to play each time we fire */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Sound)
		class USoundBase* FireSound;

	/** Sound to play each while fireing */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Sound)
		class USoundBase* FireLoopSound;
	/** FX for muzzle flash */
	UPROPERTY(EditDefaultsOnly, Category = Effects)
		UParticleSystem* MuzzleFX;

	/** Can be used to scale the muzzle fx */
	UPROPERTY(EditDefaultsOnly, Category = Effects)
		FVector MuzzleFXScale = FVector(1.f, 1.f, 1.f);

	/** Duration for the muzzle FX flash (-1 for endless)*/
	UPROPERTY(EditDefaultsOnly, Category = Effects)
		float MuzzleFXDuration = 0.1;

	/** CameraShake when fiering */
	UPROPERTY(EditDefaultsOnly, Category = Effects)
		TSubclassOf<UCameraShake> FireCameraShake;

	/** Whether automatic fire should be enabled for this weapon */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = FireMode, Replicated)
		EFireMode FireMode;

	/** Whether automatic fire should be enabled for this weapon */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = WeaponConfig)
		bool AutoFire = true;

	/** Seconds to wait for the first shot */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = WeaponConfig)
		float ChargeDuration = 0.0f;

	/** Seconds to wait between two shots */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = WeaponConfig)
		float FireInterval = 1.0f;

	/** How many shots are fired at once */
	UPROPERTY(EditDefaultsOnly, Category = WeaponConfig)
		int FireShotNum = 1;
public:
	float timeSinceLastShot = 0.f;

	UPROPERTY(BlueprintReadOnly, Replicated)
		class ABaseWeapon* Weapon;
	UPROPERTY(BlueprintReadOnly, Replicated)
		class ABaseCharacter* OwningCharacter;

	UPROPERTY(Replicated)
		EWeaponModuleState CurrentState = EWeaponModuleState::DEACTIVATED;
public:
	/* ------------- */
	/*   Functions   */
	/* ------------- */

	/* Gets called when properties like owner etc are set properly */
	virtual void Initialize() {};

	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	/* Tick that is called on server & clients*/
	virtual void TickComponent(float DeltaTime, enum ELevelTick tickType, FActorComponentTickFunction *thisTickFunction) override;

	/** If possible, changes the current state to FIRING */
	virtual bool StartFire();

	/** If currently firing, changes the current state to IDLE */
	virtual void StopFire();

	/** Fires a shot (includes animation, sound, and decreasing ammo) */
	virtual void Fire();

	/** Returns true if firing should be possible */
	virtual bool CanFire();

	/** Fires a shot (includes animation, sound, and decreasing ammo) */
	bool IsFiring();

	/** Called before ExecuteFire */
	virtual void PreExecuteFire();

	/** This function should be overriden in subclasses to implement specific fire behaviour */
	virtual void ExecuteFire() PURE_VIRTUAL(ABaseWeapon::ExecuteFire, ;);

	/** Called after ExecuteFire*/
	virtual void PostExecuteFire();

	/** Called when this module is attached to the weapon */
	virtual void OnActivate();

	/** Called when this module is detached from the weapon */
	virtual void OnDeactivate();

	/** Notifies clients about projectile fired (plays animation, sound etc.) */
	virtual void FireExecuted();

	UFUNCTION()
		void ChangeModuleState(EWeaponModuleState newState);

	UFUNCTION(NetMulticast, Reliable)
		void MulticastPlayCameraShake();
protected:
	UAudioComponent* ChargeSoundComponent;
	UAudioComponent* FireLoopSoundComponent;

	bool RecordKeyReleaseNextTick = false;
	float timeSinceStartFire = 0.f;

	bool wantsToFire = false;
};
