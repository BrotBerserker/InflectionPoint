// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Gameplay/Characters/BaseCharacter.h"
#include "Gameplay/Controllers/PlayerControllerBase.h"
#include "RadialDamageDealer.generated.h"


UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class INFLECTIONPOINT_API URadialDamageDealer : public UActorComponent {
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	URadialDamageDealer();



	/** Deals damage imideatly after delay  */
	UPROPERTY(EditDefaultsOnly, Category = DamageConfig)
		bool DealDamageOnBeginPlay = true;

	/** Deals damage imideatly after delay  */
	UPROPERTY(EditDefaultsOnly, Category = DamageConfig)
		bool DealDamageToDamageCauser = true;

	/** Delay before dealing damage */
	UPROPERTY(EditDefaultsOnly, Category = DamageConfig)
		float DamageDealDelay = 0;

	/** The base damage to apply, i.e. the damage at the origin. */
	UPROPERTY(EditDefaultsOnly, Category = DamageConfig)
		float BaseDamage = 25;

	/** Minimal dealed damage */
	UPROPERTY(EditDefaultsOnly, Category = DamageConfig)
		float MinimumDamage = 10;

	/** Radius of the full damage area, from Origin */
	UPROPERTY(EditDefaultsOnly, Category = DamageConfig)
		float DamageInnerRadius = 100;

	/** Radius of the minimum damage area, from Origin */
	UPROPERTY(EditDefaultsOnly, Category = DamageConfig)
		float DamageOuterRadius = 150;

	/** Falloff exponent of damage from DamageInnerRadius to DamageOuterRadius */
	UPROPERTY(EditDefaultsOnly, Category = DamageConfig)
		float Falloff = 1;

	/** Class that describes the damage that was done. */
	UPROPERTY(EditDefaultsOnly, Category = DamageConfig)
		TSubclassOf < class UDamageType > DamageTypeClass;

	/** Damage will not be applied to victim if there is something between the origin and the victim which blocks traces on this channel */
	UPROPERTY(EditDefaultsOnly, Category = DamageConfig, meta = (Bitmask, BitmaskEnum = "ECollisionChannel"))
		int32 DamagePreventionChannel = ECC_Visibility;

	// Called when the game starts
	virtual void BeginPlay() override;

	/** Deals RadialDamage after delay */
	UFUNCTION(BlueprintCallable)
		void DealDamage();

private:
	UFUNCTION()
		void ExecuteDealDamage(FVector location, APlayerControllerBase* controller, AActor* instigator);

	TArray<AActor*> ApplyRadialDamageWithFalloff(const FVector& Origin, AActor* DamageCauser, AController* InstigatedByController);

	bool CanHitComponent(UPrimitiveComponent* VictimComp, FVector const& Origin, ECollisionChannel TraceChannel, FHitResult& OutHitResult);
};
