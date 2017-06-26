// Copyright 1998-2016 Epic Games, Inc. All Rights Reserved.
#pragma once
#include "GameFramework/Character.h"
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


public:
	/* ---------------------- */
	/*   Blueprint Settings   */
	/* ---------------------- */

	/** Base turn rate, in deg/sec. Other scaling may affect final turn rate. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Camera)
		float BaseTurnRate = 45.f;

	/** Base look up/down rate, in deg/sec. Other scaling may affect final rate. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Camera)
		float BaseLookUpRate = 45.f;

	/** Projectile class to spawn */
	UPROPERTY(EditDefaultsOnly, Category = Projectile)
		TSubclassOf<class AInflectionPointProjectile> ProjectileClass;

	/** Debug projectile class to spawn */
	UPROPERTY(EditDefaultsOnly, Category = Projectile)
		TSubclassOf<class AInflectionPointProjectile> DebugProjectileClass;

	/** Sound to play each time we fire */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Gameplay)
		class USoundBase* FireSound;

	/** AnimMontage to play each time we fire */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Gameplay)
		class UAnimMontage* FireAnimation;

	/* Used in Multiplayer to validate Location Offsets (0< will ignore the offset) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Gameplay)
		float LocationOffsetTolerance = 5.;

	/* Used in Multiplayer to validate Rotation Offsets (0< will ignore the offset) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Gameplay)
		float RotationOffsetTolerance = -1;


public:
	/* ------------- */
	/*   Functions   */
	/* ------------- */

	/** Constructor, initializes Components */
	ABaseCharacter();

	/** Setup component attachments */
	virtual void BeginPlay();

	/** Takes damage using the MortalityProvider */
	float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, class AActor* DamageCauser) override;

	/** Fires a projectile. */
	void OnFire();

	/** Fires a debug projectile. */
	void OnDebugFire();

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

public:
	/* --------------- */
	/*  RPC Functions  */
	/* --------------- */

	/** Fires the given projectile on the Server*/
	UFUNCTION(Reliable, Server, WithValidation)
		void ServerFireProjectile(TSubclassOf<class AInflectionPointProjectile> projectileClassToSpawn, const FVector spawnLocation, const FRotator spawnRotation);

	/** Notifies Clients about projectile fired */
	UFUNCTION(Unreliable, NetMulticast)
		void MulticastProjectileFired();

	/** Updates the Camera pitch rotation on Server */
	UFUNCTION(Reliable, Server, WithValidation)
		void ServerUpdateCameraPitch(float pitch);

public:
	/* ------------------ */
	/*  Getter Functions  */
	/* ------------------ */

	/** Returns Mesh1P subobject **/
	FORCEINLINE class USkeletalMeshComponent* GetMesh1P() const { return Mesh1P; }

	/** Returns FirstPersonCameraComponent subobject **/
	FORCEINLINE class UCameraComponent* GetFirstPersonCameraComponent() const { return FirstPersonCameraComponent; }

	/** Returns Mesh3P subobject **/
	FORCEINLINE class USkeletalMeshComponent* GetMesh3P() const { return Mesh3P; }

};

