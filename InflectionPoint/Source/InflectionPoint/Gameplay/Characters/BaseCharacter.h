// Copyright 1998-2016 Epic Games, Inc. All Rights Reserved.
#pragma once
#include "GameFramework/Character.h"
#include "Gamemodes/TDMPlayerStateBase.h"
#include "BaseCharacter.generated.h"


class UInputComponent;

UCLASS(config = Game)
class ABaseCharacter : public ACharacter {
	GENERATED_BODY()

public:
	/* -------------- */
	/*   Components   */
	/* -------------- */

	/** Pawn mesh: 1st person view (arms; seen only by self) */
	UPROPERTY(VisibleDefaultsOnly, Category = Mesh)
		class USkeletalMeshComponent* Mesh1P;

	/** Gun mesh: 1st person view (seen only by self) */
	UPROPERTY(VisibleDefaultsOnly, Category = Mesh)
		class USkeletalMeshComponent* FP_Gun;

	/** Location on gun mesh where projectiles should spawn. */
	UPROPERTY(VisibleDefaultsOnly, Category = Mesh)
		class USceneComponent* FP_MuzzleLocation;

	/** Pawn mesh: 3rd person view (completed body; seen only by others) */
	UPROPERTY(VisibleDefaultsOnly, Category = Mesh)
		class USkeletalMeshComponent* Mesh3P;

	/** Gun mesh: 3rd person view (seen only by others) */
	UPROPERTY(VisibleDefaultsOnly, Category = Mesh)
		class USkeletalMeshComponent* TP_Gun;

	/** First person camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
		class UCameraComponent* FirstPersonCameraComponent;

	/** MortalityProvider which holds our HP */
	UPROPERTY(VisibleDefaultsOnly, Category = Mesh)
		class UMortalityProvider* MortalityProvider;

	/** Timeline used to show the materialize effect */
	UPROPERTY()
		UTimelineComponent* MaterializeTimeline;

public:
	/* ---------------------- */
	/*    Editor Settings     */
	/* ---------------------- */

	/** Projectile class to spawn */
	UPROPERTY(EditDefaultsOnly, Category = Projectile)
		TSubclassOf<class AInflectionPointProjectile> ProjectileClass;

	/** Debug projectile class to spawn */
	UPROPERTY(EditDefaultsOnly, Category = Projectile)
		TSubclassOf<class AInflectionPointProjectile> DebugProjectileClass;

	/** Minimum a player has to wait between to shots */
	UPROPERTY(EditDefaultsOnly, Category = Projectile)
		float DelayBetweenShots = 0.2f;

	/** Number of shots per clip */
	UPROPERTY(EditDefaultsOnly, Category = Projectile)
		int MaxAmmo = 7;

	/** Sound to play each time we fire */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Projectile)
		class USoundBase* FireSound;

	/** AnimMontage to play each time we fire */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Projectile)
		class UAnimMontage* FireAnimation;

	/** AnimMontage to play when reloading */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Projectile)
		class UAnimMontage* ReloadAnimation1P;

	/** Determines the maximum walk speed when sprinting */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Movement)
		int SprintSpeed = 900;

	/** Determines the materialize amount over time when playing the materialize animation */
	UPROPERTY(EditAnywhere, Category = Materialize)
		UCurveFloat* MaterializeCurve;

	/** Material to use for Mesh3P and Mesh1P after the materialize animation has finished */
	UPROPERTY(EditAnywhere, Category = Materialize)
		UMaterialInstance* BodyMaterialAfterMaterialize;

	/** Material to use for the gun after the materialize animation has finished */
	UPROPERTY(EditAnywhere, Category = Materialize)
		UMaterial* GunMaterialAfterMaterialize;

	/** One of these animations will be played when the character dies */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Mesh)
		TArray<UAnimationAsset*> DeathAnimations;

	/* Max distance between client side and server side locations during RPC validation */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Gameplay)
		float LocationOffsetTolerance = 5.;

	/* Max differnce between client side and server side rotations during RPC validation */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Gameplay)
		float RotationOffsetTolerance = -1;

	/* Draw arrows when shots are fired */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Debug)
		bool DrawDebugArrows = false;

	/* Debug arrow color */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Debug)
		FColor DebugArrowColor;

public:
	/* Fired when this character receives damage from a certain direction */
	UFUNCTION(BlueprintImplementableEvent, Category = "InflectionPoint")
		void OnDirectionalDamageReceived(FVector direction, float damage);

public:
	/* ------------- */
	/*   Functions   */
	/* ------------- */

	/** Constructor, initializes Components */
	ABaseCharacter();

	/** Setup component attachments */
	virtual void BeginPlay();

	virtual void Tick(float DeltaTime) override;

	/** Called when this character is restartet (e.g. by being possessed), fires OnRestart */
	virtual void Restart() override;

	/** Fired when this character is restartet (e.g. by being possessed) */
	UFUNCTION(BlueprintImplementableEvent)
		void OnRestart();

	/** Returns true if Initialize() can be called (e.g. checks if the PlayerState is not null) */
	virtual bool IsReadyForInitialization() PURE_VIRTUAL(ABaseCharacter::IsReadyForInitialization, return false;);

	/** Initializes this character */
	virtual void Initialize() PURE_VIRTUAL(ABaseCharacter::Initialize, ;);

	/** Fired after Initialize() has been called */
	UFUNCTION(BlueprintImplementableEvent)
		void OnInitialized();

	/** Applies the team color to Mesh3P and Mesh1P */
	UFUNCTION(BlueprintCallable)
		void ApplyPlayerColor(ATDMPlayerStateBase* state);

	/** Applies the team color to Mesh3P and Mesh1P via Multicast */
	UFUNCTION(BlueprintCallable, NetMulticast, Unreliable)
		void MulticastApplyPlayerColor(ATDMPlayerStateBase* state);

	/** Shows a spawn animation using Materialize effects */
	UFUNCTION(BlueprintCallable)
		void ShowSpawnAnimation();

	/** Shows a spawn animation using Materialize effects via Multicast */
	UFUNCTION(BlueprintCallable, NetMulticast, Unreliable)
		void MulticastShowSpawnAnimation();

	/** Called every tick during the materialize animation */
	UFUNCTION()
		void MaterializeCallback(float value);

	/** Called when the materialize animation has finished */
	UFUNCTION()
		void MaterializeFinishCallback();

	/** Takes damage using the MortalityProvider */
	float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, class AActor* DamageCauser) override;

	/** Fires a projectile. */
	void Fire();

	/** Fires a debug projectile. */
	void DebugFire();

	/** Event fired when CurrentAmmo changes */
	UFUNCTION(BlueprintImplementableEvent)
		void OnAmmoChanged();

	/** Enables sprint and starts sprinting if all sprinting conditions are met */
	void EnableSprint();

	/** Disables sprint, stops sprinting if currently sprinting */
	void DisableSprint();

	/** Returns true if the player is not sprinting and should start sprinting */
	bool ShouldStartSprinting(float ForwardMovement);

	/** Returns true if the player is sprinting and should stop sprinting */
	bool ShouldStopSprinting(float ForwardMovement);

	/** Starts sprinting (increases max speed) */
	void StartSprinting();

	/** Stops sprinting (decreases max speed) */
	void StopSprinting();

	/** Handles moving forward/backward */
	void MoveForward(float val);

	/** Handles stafing movement, left and right */
	void MoveRight(float val);

	/**
	* Called via input to turn at a given rate.
	* @param Rate	This is a normalized rate, i.e. 1.0 means 100% of desired turn rate
	*/
	void TurnAtRate(float rate);

	/**
	* Called via input to turn look up/down at a given rate.
	* @param Rate	This is a normalized rate, i.e. 1.0 means 100% of desired turn rate
	*/
	void LookUpAtRate(float rate);

	/** Returns the location at which a projectile should spawn */
	FVector GetProjectileSpawnLocation();

	/** Returns the rotation with which a projectile should spawn */
	FRotator GetProjectileSpawnRotation();

	/** Draws an arrow indicating the current position and camera direction */
	void DrawDebugArrow();

	/** Fires the given projectile */
	virtual void FireProjectile(TSubclassOf<AInflectionPointProjectile> &projectileClassToSpawn);

	/** Stops firing */
	virtual void StopFire();

public:
	/* --------------- */
	/*  RPC Functions  */
	/* --------------- */

	/** Fires the given projectile on the Server */
	UFUNCTION(Reliable, Server, WithValidation)
		void ServerFireProjectile(TSubclassOf<class AInflectionPointProjectile> projectileClassToSpawn);

	/** Stops firing on the Server */
	UFUNCTION(Reliable, Server, WithValidation)
		void ServerStopFire();

	/** Notifies Clients about projectile fired */
	UFUNCTION(Unreliable, NetMulticast)
		void MulticastProjectileFired();

	/** Reloads the weapon by updating CurrentAmmo, assumes that the animation has already been played */
	UFUNCTION(Reliable, Server, WithValidation, BlueprintCallable)
		void ServerReload();

	/** Tells everyone that this character has reloaded */
	UFUNCTION(Unreliable, NetMulticast)
		void MulticastReloaded();

	/** Starts sprinting via RPC */
	UFUNCTION(Reliable, Server, WithValidation)
		void ServerStartSprinting();

	/** Stops sprinting via RPC */
	UFUNCTION(Reliable, Server, WithValidation)
		void ServerStopSprinting();

	/** Plays a death animation, disables input and collisions */
	UFUNCTION(Unreliable, NetMulticast, BlueprintCallable)
		void MulticastOnDeath();

	/** Shows a death camera perspective */
	UFUNCTION(Unreliable, Client, BlueprintCallable)
		void ClientOnDeath();

	/** Updates the Camera pitch rotation on Server */
	UFUNCTION(Reliable, Server, WithValidation)
		void ServerUpdateCameraPitch(float pitch);

	/** Updates the Camera pitch rotation on Clients */
	UFUNCTION(Reliable, NetMulticast)
		void MulticastUpdateCameraPitch(float pitch);

public:
	/* ---------------------- */
	/*  Replicated Variables  */
	/* ---------------------- */
	UPROPERTY(Replicated, BlueprintReadWrite)
		int CurrentAmmo;

private:
	bool initialized = false;

	bool sprintEnabled = false;
	int walkSpeed;

	float LastShotTimeStamp;

	UMaterialInstanceDynamic* DynamicBodyMaterial;
	UMaterialInstanceDynamic* DynamicGunMaterial;

};

