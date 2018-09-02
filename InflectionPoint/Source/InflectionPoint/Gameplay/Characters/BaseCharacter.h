// Copyright 1998-2016 Epic Games, Inc. All Rights Reserved.
#pragma once
#include "GameFramework/Character.h"
#include "Gamemodes/TDMPlayerStateBase.h"
#include "Gameplay/CharacterInfoProvider.h"
#include "Gameplay/Weapons/BaseWeapon.h"
#include "Gameplay/Controllers/PlayerControllerBase.h"
#include "WidgetComponent.h"
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

	/** Pawn mesh: 3rd person view (completed body; seen only by others) */
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = Mesh)
		class USkeletalMeshComponent* Mesh3P;

	/** First person camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
		class UCameraComponent* FirstPersonCameraComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
		class UWeaponInventory* WeaponInventory;

	/** MortalityProvider which holds our HP */
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = Mesh)
		class UMortalityProvider* MortalityProvider;

	/** Timeline used to show the materialize effect */
	UPROPERTY()
		UTimelineComponent* MaterializeTimeline;

	UPROPERTY(BlueprintReadOnly)
		class UCharacterInfoProvider* CharacterInfoProvider;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly)
		class UWidgetComponent* CharacterHeadDisplay;
public:
	/* ---------------------- */
	/*    Editor Settings     */
	/* ---------------------- */

	/** Determines the maximum walk speed when sprinting */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Movement)
		int SprintSpeed = 900;

	/** Determines the maximum walk speed when walking normaly */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Movement)
		int WalkSpeed = 600;

	/** Determines the materialize amount over time when playing the materialize animation */
	UPROPERTY(EditAnywhere, Category = Materialize)
		UCurveFloat* MaterializeCurve;

	/** Material to use for Mesh3P and Mesh1P after the materialize animation has finished */
	UPROPERTY(EditAnywhere, Category = Materialize)
		UMaterialInstance* BodyMaterialAfterMaterialize;

	/** One of these animations will be played when the character dies */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Mesh)
		TArray<UAnimationAsset*> DeathAnimations;

	/* Max distance between client side and server side locations during RPC validation */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Gameplay)
		float LocationOffsetTolerance = 5.;

	/* Max differnce between client side and server side rotations during RPC validation */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Gameplay)
		float RotationOffsetTolerance = -1;

	/* Debug arrow color */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Debug)
		FColor DebugArrowColor;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Material)
		FName TeamColorMaterialParameterName = FName("BodyMetalColor");

public:
	/* Fired when this character receives damage from a certain direction */
	UFUNCTION(BlueprintImplementableEvent, Category = "InflectionPoint")
		void OnDirectionalDamageReceived(FVector direction, float damage, TSubclassOf<class UDamageType> DamageType);

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

	virtual void Destroyed() override;

	/** Fired when this character is restartet (e.g. by being possessed) */
	UFUNCTION(BlueprintImplementableEvent)
		void OnRestart();

	/** Returns true if Initialize() can be called (e.g. checks if the PlayerState is not null) */
	virtual bool IsReadyForInitialization();

	/** Initializes this character */
	virtual void Initialize();

	/** Fired after Initialize() has been called */
	UFUNCTION(BlueprintImplementableEvent)
		void OnInitialized();

	/** Applies the team color to Mesh3P and Mesh1P */
	UFUNCTION(BlueprintCallable)
		void ApplyTeamColor(ATDMPlayerStateBase* state);

	/** Applies the team color to Mesh3P and Mesh1P via Multicast */
	UFUNCTION(BlueprintCallable, NetMulticast, Unreliable)
		void MulticastApplyTeamColor(ATDMPlayerStateBase* state);

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

	/** Enables sprint and starts sprinting if all sprinting conditions are met */
	UFUNCTION(BlueprintCallable)
		void EnableSprint();

	/** Disables sprint, stops sprinting if currently sprinting */
	UFUNCTION(BlueprintCallable)
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
	UFUNCTION(BlueprintCallable)
		void MoveForward(float val);

	/** Handles stafing movement, left and right */
	void MoveRight(float val);

	/** These two are needed because sprinting is handled on client side */
	void StartFire();

	UFUNCTION(BlueprintCallable)
		void StopFire();

	/** Start/stop aiming has to be done on clients and server to ensure smooth animations */
	void StartAiming();

	UFUNCTION(BlueprintCallable)
		void StopAiming();

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

	/** Draws an arrow indicating the current position and camera direction */
	void DrawDebugArrow();

	UFUNCTION(BlueprintCallable)
		bool IsAReplay();

	UFUNCTION(BlueprintCallable)
		bool IsInSameTeamAs(class ABaseCharacter* character);

	/** If this Character is alive or not */
	UFUNCTION(BlueprintCallable)
		bool IsAlive();

	UFUNCTION(BlueprintCallable)
		bool IsInSameTeamAsLocalPlayer();

	UFUNCTION(BlueprintCallable)
		void InitCharacterHeadDisplay();

	UFUNCTION(BlueprintCallable)
		class UCharacterHeadDisplayBase* GetCharacterHeadDisplay();
public:
	/* --------------- */
	/*  RPC Functions  */
	/* --------------- */

	/** Starts firing the currently equipped weapon */
	UFUNCTION(BlueprintCallable, Server, Reliable, WithValidation)
		virtual void ServerFireOnce();

	/** Starts firing the currently equipped weapon */
	UFUNCTION(BlueprintCallable, Server, Reliable, WithValidation)
		virtual void ServerStartFire();

	/** Stops firing the currently equipped weapon */
	UFUNCTION(BlueprintCallable, Server, Reliable, WithValidation)
		virtual void ServerStopFire();

	/** Starts aiming, which changes some animations and increases the weapon's precision */
	UFUNCTION(Server, Reliable, WithValidation)
		virtual void ServerStartAiming();

	/** Stops aiming, which changes some animations and decreases the weapon's precision */
	UFUNCTION(Server, Reliable, WithValidation)
		virtual void ServerStopAiming();

	/** Shows the aiming animations an all clients */
	UFUNCTION(NetMulticast, Reliable)
		virtual void MulticastStartAiming();

	/** Stops the aiming animations an all clients */
	UFUNCTION(NetMulticast, Reliable)
		virtual void MulticastStopAiming();

	/** Reloads the currently equipped weapon */
	UFUNCTION(Server, Reliable, WithValidation, BlueprintCallable)
		void ServerReload();

	UFUNCTION(Server, Reliable, WithValidation)
		void ServerEquipNextWeapon();

	UFUNCTION(Server, Reliable, WithValidation)
		void ServerEquipPreviousWeapon();

	UFUNCTION(Server, Reliable, WithValidation, BlueprintCallable)
		void ServerEquipSpecificWeapon(int index);

	UFUNCTION(Server, Reliable, WithValidation)
		void ServerEquipRandomWeapon();

	UFUNCTION(NetMulticast, Reliable)
		void MulticastWeaponChanged(ABaseWeapon* newWeapon, ABaseWeapon* oldWeapon);

	/** Starts sprinting via RPC */
	UFUNCTION(Reliable, Server, WithValidation, BlueprintCallable)
		void ServerStartSprinting();

	/** Stops sprinting via RPC */
	UFUNCTION(Reliable, Server, WithValidation, BlueprintCallable)
		void ServerStopSprinting();

	/** Plays a death animation, disables input and collisions */
	UFUNCTION(Reliable, NetMulticast, BlueprintCallable)
		void MulticastOnDeath();

	/** Plays an animation on the Mesh3P */
	UFUNCTION(Reliable, NetMulticast, BlueprintCallable)
		void MulticastPlay3PAnimation(UAnimationAsset* animation);

	/** Hides Mesh1P and shows Mesh3P for the death camera */
	UFUNCTION(Reliable, Client, BlueprintCallable)
		void ClientOnDeath();

	/** Updates the Camera pitch rotation on Server */
	UFUNCTION(Reliable, Server, WithValidation)
		void ServerUpdateCameraPitch(float pitch);

	/** Updates the Camera pitch rotation on Clients */
	UFUNCTION(Reliable, NetMulticast)
		void MulticastUpdateCameraPitch(float pitch);

public:
	UPROPERTY(BlueprintReadWrite, ReplicatedUsing = OnRep_CurrentWeapon)
		ABaseWeapon* CurrentWeapon;

	UFUNCTION()
		void OnRep_CurrentWeapon(ABaseWeapon* OldWeapon);

	void EquipWeapon(ABaseWeapon* NewWeapon, ABaseWeapon* OldWeapon = NULL);

	UPROPERTY(BlueprintReadWrite)
		bool IsAiming = false;
private:
	void UpdateFieldOfView(float DeltaTime);

	void ApplyColorToMaterials(UMeshComponent* mesh, FLinearColor color);

private:
	bool initialized = false;

	/** True if the player has pressed shift and wants to sprint */
	bool sprintEnabled = false;

	/** False if the player is not allowed to sprint (e.g. when firing a weapon) */
	bool sprintAllowed = true;

	UMaterialInstanceDynamic* DynamicBodyMaterial;
};

