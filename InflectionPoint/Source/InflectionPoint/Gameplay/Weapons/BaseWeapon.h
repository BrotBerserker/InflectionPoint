// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Gameplay/Weapons/InflectionPointProjectile.h"
#include "Gameplay/Weapons/FireModules/BaseWeaponModule.h"
#include "Blueprint/UserWidget.h"
#include "BaseWeapon.generated.h"

class ABaseCharacter;
class UPlayerStateRecorder;

UENUM(BlueprintType)
enum EWeaponState {
	IDLE,
	RELOADING,
	EQUIPPING,
	CHARGING, // TODO: move Charging & fiering state to modules
	FIRING
};

UENUM(Blueprintable)
enum class EFireMode : uint8 {
	Primary = 0,
	Secondary = 0,
};

USTRUCT(BlueprintType)
struct FBaseWeaponModus {
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadonly)
		TSubclassOf<UBaseWeaponModule> PrimaryModuleClass;
	UPROPERTY(BlueprintReadonly)
		UBaseWeaponModule* PrimaryModule;

	UPROPERTY(EditAnywhere, BlueprintReadonly)
		TSubclassOf<UBaseWeaponModule> SecondaryModuleClass;
	UPROPERTY(BlueprintReadonly)
		UBaseWeaponModule* SecondaryModule;
	
	/* Weather you can use primary & secondary fire modules async */
	UPROPERTY(EditAnywhere, BlueprintReadonly)
		bool IsAsync = true;
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

	/** Idle animation (without aiming) for 1P */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation|1P")
		class UAnimationAsset* IdleAnimation1P;

	/** Walk animation (without aiming) for 1P */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation|1P")
		class UAnimationAsset* WalkAnimation1P;

	/** Sprint animation for 1P */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation|1P")
		class UAnimationAsset* SprintAnimation1P;

	/** Jump start animation for 1P */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation|1P")
		class UAnimationAsset* JumpStartAnimation1P;

	/** Jump loop animation for 1P */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation|1P")
		class UAnimationAsset* JumpLoopAnimation1P;

	/** Jump end animation for 1P */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation|1P")
		class UAnimationAsset* JumpEndAnimation1P;

	/** Idle animation (without aiming) for 3P */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation|3P")
		class UAnimationAsset* IdleAnimation3P;

	/** AnimMontage to play each time we fire */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation|1P")
		class UAnimMontage* FireAnimation;

	/** AnimMontage to play on 1p weapon when firing */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation|1P")
		class UAnimationAsset* FireAnimationWeapon1P;

	/** AnimMontage to play on 1P hands when reloading */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation|1P")
		class UAnimMontage* ReloadAnimation1P;

	/** AnimMontage to play on 3p hands when reloading */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation|3P")
		class UAnimMontage* ReloadAnimation3P;

	/** Animation to play on 1p weapon when reloading */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation|1P")
		class UAnimationAsset* ReloadAnimationWeapon1P;

	/** Idle Animation to play on 1p hands when aiming */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation|1P")
		class UAnimationAsset* AimAnimation1PIdle;

	/** Walk Animation to play on 1p hands when aiming */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation|1P")
		class UAnimationAsset* AimAnimation1PWalk;

	/** JumpStart Animation to play on 1p hands when aiming */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation|1P")
		class UAnimationAsset* AimAnimation1PJumpStart;

	/** JumpEnd Animation to play on 1p hands when aiming */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation|1P")
		class UAnimationAsset* AimAnimation1PJumpEnd;

	/** AnimMontage to play on Mesh1P when this weapon is equipped */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation|1P")
		class UAnimMontage* EquipAnimation1P;

	/** AnimMontage to playon Mesh3P when this weapon is equipped */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation|3P")
		class UAnimMontage* EquipAnimation3P;

	/** List of fire Modi for this weapon */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Replicated, Category = WeaponConfig)
		TArray<FBaseWeaponModus> WeaponModi;

	/** Number of shots per clip */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = WeaponConfig)
		int ClipSize = 7;

	/** Current amount of munition (with CurrentAmmoInClip included) */
	UPROPERTY(Replicated, EditAnywhere, BlueprintReadWrite, Category = WeaponConfig)
		int CurrentAmmo = -1;

	/** Max amount of munition (with CurrentAmmoInClip included) */
	UPROPERTY(Replicated, EditAnywhere, BlueprintReadWrite, Category = WeaponConfig)
		int MaxAmmo = -1;

	/** Delay before firstshot after equip */
	UPROPERTY(EditDefaultsOnly, Category = WeaponConfig)
		float EquipDelay = 0.2f;

	/** The FieldOfView when Aiming with the Weapon (for a zoom effect) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = WeaponConfig)
		float AimFieldOfView = 75.f;

	/** The FieldOfView when Aiming with the Weapon (for a zoom effect) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "WeaponConfig|AI")
		FRuntimeFloatCurve AISuitabilityWeaponRangeCurve;

	/** If true, the crosshair will be hidden when aiming with this weapon */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = WeaponConfig)
		bool HideCrosshairWhenAiming = true;

	/** Widget that gets displayed when aiming */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = WeaponConfig)
		TSubclassOf<UUserWidget> AimDisplayWidget;

	/** Hides Weapon when aiming */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = WeaponConfig)
		bool HideWeaponWhenAiming = false;

	/** Widget that shows the crosshair */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = WeaponConfig)
		TSubclassOf<UUserWidget> CrosshairDisplayWidget;
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

	void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

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
	virtual void FireOnce(EFireMode mode);

	/** If possible, changes the current state to FIRING */
	virtual void StartFire(EFireMode mode);

	/** If currently firing, changes the current state to IDLE */
	virtual void StopFire(EFireMode mode);

	/** Fires a shot (includes animation, sound, and decreasing ammo) */
	virtual void Fire();

	/** Returns true if firing should be possible */
	virtual bool CanFire();

	/** Called before ExecuteFire */
	virtual void PreExecuteFire();

	/** This function should be overriden in subclasses to implement specific fire behaviour */
	virtual void ExecuteFire() {};

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

	/** Used to fire OnStateChanged as multicast */
	UFUNCTION(NetMulticast, Reliable)
		void MulticastStateChanged(EWeaponState NewState);

	/** Fired when the weapon state has been changed */
	UFUNCTION(BlueprintImplementableEvent)
		void OnStateChanged(EWeaponState NewState);
public:
	UPROPERTY(BlueprintReadWrite, Replicated)
		ABaseCharacter* OwningCharacter;

	UPROPERTY(Replicated, BlueprintReadWrite)
		int CurrentAmmoInClip;

	UPROPERTY(Replicated, BlueprintReadOnly)
		int CurrentWeaponModusIndex = 0;

public:
	FScriptDelegate AnimationNotifyDelegate;
protected:
	/** Spawns the sound if not existent and starts or stops it*/
	void TogglePersistentSoundFX(UAudioComponent*& component, class USoundBase* soundClass, bool shouldPlay, float fadeOut = 0.2);

	UPROPERTY(Replicated)
		TEnumAsByte<EWeaponState> CurrentState = EWeaponState::IDLE;

	UPlayerStateRecorder* Recorder;

	UPROPERTY(Replicated) // gets set to true if weapon fires
		bool shouldPlayFireFX = false;

	float timeSinceLastShot = 0.f;
	float timeSinceStartFire = 0.f;

	bool wantsToFire = false;

	bool equipped = false;
	bool RecordKeyReleaseNextTick = false;

	void UpdateEquippedState(bool equipped);

	void SetupWeaponModi();

	UBaseWeaponModule* CurrentWeaponModule; // NOPE: remove to enable async firemodes
	UBaseWeaponModule* CreateWeaponModule(TSubclassOf<UBaseWeaponModule> clazz);
	FBaseWeaponModus& GetCurrentWeaponModus();

	UFUNCTION()
		void ChangeWeaponState(EWeaponState newState);
private:
	UAudioComponent* ChargeSoundComponent;
	UAudioComponent* FireLoopSoundComponent; 
};