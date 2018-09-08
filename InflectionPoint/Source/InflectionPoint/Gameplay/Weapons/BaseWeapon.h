// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Gameplay/Weapons/InflectionPointProjectile.h"
#include "Blueprint/UserWidget.h"
#include "BaseWeapon.generated.h"

class ABaseCharacter;
class UPlayerStateRecorder;

UENUM(BlueprintType)
enum EWeaponState {
	IDLE,
	RELOADING,
	EQUIPPING,
	FIRING
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnFireExecutedDelegate);

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

	/** Location on Mesh1P where projectiles and effects should spawn if the player is aiming. */
	UPROPERTY(VisibleDefaultsOnly, Category = Mesh)
		class USceneComponent* FP_Aim_MuzzleLocation;

	/** Location on Mesh3P where effects should spawn. */
	UPROPERTY(VisibleDefaultsOnly, Category = Mesh)
		class USceneComponent* TP_MuzzleLocation;

public:
	/* ---------------------- */
	/*    Editor Settings     */
	/* ---------------------- */

	/** Sound to play each time we fire */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = GUI)
		UTexture2D* WeaponTexture;

	/** Sound to play when no Ammo is left */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Sound)
		class USoundBase* NoAmmoSound;

	/** Sound to play each time we fire */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Sound)
		class USoundBase* FireSound;

	/** AnimMontage to play each time we fire */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Animation)
		class UAnimMontage* FireAnimation;

	/** AnimMontage to play on 1p weapon when firing */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Animation)
		class UAnimationAsset* FireAnimationWeapon1P;

	/** AnimMontage to play on 1P hands when reloading */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Animation)
		class UAnimMontage* ReloadAnimation1P;

	/** AnimMontage to play on 3p hands when reloading */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Animation)
		class UAnimMontage* ReloadAnimation3P;

	/** Animation to play on 1p weapon when reloading */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Animation)
		class UAnimationAsset* ReloadAnimationWeapon1P;

	/** Idle Animation to play on 1p hands when aiming */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation|Aiming")
		class UAnimationAsset* AimAnimation1PIdle;

	/** Walk Animation to play on 1p hands when aiming */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation|Aiming")
		class UAnimationAsset* AimAnimation1PWalk;

	/** JumpStart Animation to play on 1p hands when aiming */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation|Aiming")
		class UAnimationAsset* AimAnimation1PJumpStart;

	/** JumpEnd Animation to play on 1p hands when aiming */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation|Aiming")
		class UAnimationAsset* AimAnimation1PJumpEnd;

	/** FX for muzzle flash */
	UPROPERTY(EditDefaultsOnly, Category = Effects)
		UParticleSystem* MuzzleFX;

	/** Can be used to scale the muzzle fx */
	UPROPERTY(EditDefaultsOnly, Category = Effects)
		FVector MuzzleFXScale = FVector(1.f, 1.f, 1.f);

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
		int ClipSize = 7;

	/** Current amount of munition (with CurrentAmmoInClip included) */
	UPROPERTY(Replicated, EditAnywhere, BlueprintReadWrite, Category = WeaponConfig)
		int CurrentAmmo = -1;

	/** Max amount of munition (with CurrentAmmoInClip included) */
	UPROPERTY(Replicated, EditAnywhere, BlueprintReadWrite, Category = WeaponConfig)
		int MaxAmmo = -1;

	/** Whether automatic fire should be enabled for this weapon */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = WeaponConfig)
		bool AutoFire = true;

	/** Seconds to wait between two shots */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = WeaponConfig)
		float FireInterval = 1.0f;

	/** Delay before reloading */
	UPROPERTY(EditDefaultsOnly, Category = WeaponConfig)
		float ReloadDelay = 0;

	/** Delay before firstshot after equip */
	UPROPERTY(EditDefaultsOnly, Category = WeaponConfig)
		float EquipDelay = 0.2f;

	/** How many shots are fired at once */
	UPROPERTY(EditDefaultsOnly, Category = WeaponConfig)
		int FireShotNum = 1;

	/** If true, the crosshair will be hidden when aiming with this weapon */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = WeaponConfig)
		bool HideCrosshairWhenAiming = true;

	/** The FieldOfView when Aiming with the Weapon (for a zoom effect) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = WeaponConfig)
		float AimFieldOfView = 75.f;

	/** Widget that gets displayed when aiming */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = WeaponConfig)
		TSubclassOf<UUserWidget> AimDisplayWidget;

	/** Hides Weapon when aiming */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = WeaponConfig)
		bool HideWeaponWhenAiming = false;

	/** Widget that shows the crosshair */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = WeaponConfig)
		TSubclassOf<UUserWidget> CrosshairDisplayWidget;


	/** The FieldOfView when Aiming with the Weapon (for a zoom effect) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "WeaponConfig|AI")
		FRuntimeFloatCurve AISuitabilityWeaponRangeCurve;
public:
	/* ------------- */
	/*    Events     */
	/* ------------- */
	UPROPERTY(BlueprintAssignable)
		FOnFireExecutedDelegate OnFireExecuted;
public:
	/* ------------- */
	/*   Functions   */
	/* ------------- */

	/** Constructor, initializes components */
	ABaseWeapon();

	/** Initializes variables and attachments */
	virtual void BeginPlay() override;

	void SetupReferences();

	virtual bool IsReadyForInitialization();

	/** Called when the Instigator is set */
	void OnRep_Instigator() override;

	/** Reattach MuzzleLocation from weapon to camera to prevent the weapon animation from moving the MuzzleLocation */
	UFUNCTION()
		void ReattachMuzzleLocation();

	/** Executes behaviour depending on the current state */
	virtual void Tick(float DeltaTime) override;

	/** If possible, fires once */
	virtual void FireOnce();

	/** If possible, changes the current state to FIRING */
	virtual void StartFire();

	/** If currently firing, changes the current state to IDLE */
	virtual void StopFire();

	/** Fires a shot (includes animation, sound, and decreasing ammo) */
	virtual void Fire();

	/** Called before ExecuteFire */
	virtual void PreExecuteFire();

	/** This function should be overriden in subclasses to implement specific fire behaviour */
	virtual void ExecuteFire() PURE_VIRTUAL(ABaseWeapon::ExecuteFire, ;);

	/** Called after ExecuteFire*/
	virtual void PostExecuteFire();

	/** Notify Weapon that it is aiming  */
	virtual void StartAiming();

	/** Notify Weapon that it is not aiming anymore */
	virtual void StopAiming();

	/** Plays the reload animation (only if the weapon doesn't have max ammo) */
	UFUNCTION()
		virtual void Reload();

	/** Plays the reload animation on all clients */
	UFUNCTION(NetMulticast, Reliable)
		virtual void MulticastPlayReloadAnimation();
	
	UFUNCTION(Server, Reliable, WithValidation, BlueprintCallable)
	virtual void ServerIncreaseCurrentAmmo(int amount);

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

	/* Spawns the No Ammo Sound */
	UFUNCTION(NetMulticast, Reliable)
		void MulticastSpawnNoAmmoSound();

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

	UFUNCTION(BlueprintCallable)
		EWeaponState GetCurrentWeaponState();


	/** Value between 0 and 1 that tells a AI how suitabe the Weapon is */
	UFUNCTION(BlueprintCallable)
		virtual float GetAIWeaponSuitability(ABaseCharacter* shooter, AActor* victim);

public:
	UPROPERTY(BlueprintReadWrite, Replicated)
		ABaseCharacter* OwningCharacter;

	UPROPERTY(Replicated, BlueprintReadWrite)
		int CurrentAmmoInClip;

protected:
	UPROPERTY(Replicated)
		TEnumAsByte<EWeaponState> CurrentState = EWeaponState::IDLE;

	UPlayerStateRecorder* Recorder;

	float timeSinceLastShot = 0.f;

	FScriptDelegate AnimationNotifyDelegate;
	FScriptDelegate AnimationEndDelegate;

	bool equipped = false;
	bool RecordKeyReleaseNextTick = false;

	void UpdateEquippedState(bool equipped);
	
	UFUNCTION()
		void ChangeWeaponState(EWeaponState newState);

};