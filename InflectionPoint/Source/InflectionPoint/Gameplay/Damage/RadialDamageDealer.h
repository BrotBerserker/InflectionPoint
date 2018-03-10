// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Serialization/MemoryWriter.h"
#include "Serialization/CustomVersion.h"
#include "Serialization/ObjectAndNameAsStringProxyArchive.h"
#include "Misc/PackageName.h"
#include "Misc/EngineVersion.h"
#include "GameFramework/DamageType.h"
#include "GameFramework/Pawn.h"
#include "WorldCollision.h"
#include "SceneView.h"
#include "Components/PrimitiveComponent.h"
#include "Serialization/MemoryReader.h"
#include "UObject/Package.h"
#include "Audio.h"
#include "GameFramework/WorldSettings.h"
#include "Engine/CollisionProfile.h"
#include "ParticleHelper.h"
#include "Particles/ParticleSystemComponent.h"
#include "Engine/LevelStreaming.h"
#include "Engine/LocalPlayer.h"
#include "ActiveSound.h"
#include "DrawDebugHelpers.h"
#include "EngineUtils.h"
#include "AudioDevice.h"
#include "SaveGameSystem.h"
#include "DVRStreaming.h"
#include "PlatformFeatures.h"
#include "GameFramework/Character.h"
#include "Sound/SoundBase.h"
#include "Sound/DialogueWave.h"
#include "GameFramework/SaveGame.h"
#include "PhysicalMaterials/PhysicalMaterial.h"
#include "Components/DecalComponent.h"
#include "Components/ForceFeedbackComponent.h"
#include "LandscapeProxy.h"
#include "Logging/MessageLog.h"
#include "Components/HierarchicalInstancedStaticMeshComponent.h"
#include "PhysicsEngine/PhysicsSettings.h"
#include "PhysicsEngine/BodySetup.h"
#include "EngineStats.h"

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

	TArray<AActor*> ApplyRadialDamageWithFalloff(const UObject* WorldContextObject, float BaseDamage, float MinimumDamage, const FVector& Origin, float DamageInnerRadius, float DamageOuterRadius, float DamageFalloff, TSubclassOf<class UDamageType> DamageTypeClass, AActor* DamageCauser, AController* InstigatedByController, ECollisionChannel DamagePreventionChannel);

	bool CanHitComponent(UPrimitiveComponent* VictimComp, FVector const& Origin, ECollisionChannel TraceChannel, FHitResult& OutHitResult);
};
