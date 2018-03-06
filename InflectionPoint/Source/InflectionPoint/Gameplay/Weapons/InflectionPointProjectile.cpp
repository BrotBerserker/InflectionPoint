// Copyright 1998-2016 Epic Games, Inc. All Rights Reserved.

#include "InflectionPoint.h"
#include "InflectionPointProjectile.h"
#include "Gameplay/Characters/BaseCharacter.h"
#include "Gameplay/Controllers/PlayerControllerBase.h"
#include "GameFramework/ProjectileMovementComponent.h"

AInflectionPointProjectile::AInflectionPointProjectile() {
	// Use a sphere as a simple collision representation
	CollisionComp = CreateDefaultSubobject<USphereComponent>(TEXT("SphereComp"));
	CollisionComp->InitSphereRadius(5.0f);
	CollisionComp->BodyInstance.SetCollisionProfileName("Projectile");
	CollisionComp->OnComponentHit.AddDynamic(this, &AInflectionPointProjectile::OnHit);		// set up a notification for when this component hits something blocking

	// Players can't walk on it
	CollisionComp->SetWalkableSlopeOverride(FWalkableSlopeOverride(WalkableSlope_Unwalkable, 0.f));
	CollisionComp->CanCharacterStepUpOn = ECB_No;

	CollisionDamageDealer = CreateDefaultSubobject<UCollisionDamageDealer>(TEXT("CollisionDamageDealer"));

	// Set as root component
	RootComponent = CollisionComp;

	// Use a ProjectileMovementComponent to govern this projectile's movement
	ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileComp"));
	ProjectileMovement->UpdatedComponent = CollisionComp;
	ProjectileMovement->InitialSpeed = 3000.f;
	ProjectileMovement->MaxSpeed = 3000.f;
	ProjectileMovement->bRotationFollowsVelocity = true;
	ProjectileMovement->bShouldBounce = true;

	DebugLineDrawer = CreateDefaultSubobject<UDebugLineDrawer>(TEXT("DebugLineDrawer"));

	// Die after 3 seconds by default
	InitialLifeSpan = 3.0f;
}

void AInflectionPointProjectile::BeginPlay() {
	Super::BeginPlay();

	startPos = GetActorLocation();

	// We can't do this in the constructor because CollisionDamageDealer->OnDamageHit is not yet initialized then
	CollisionDamageDealer->OnDamageHit.AddDynamic(this, &AInflectionPointProjectile::OnDamageHit);
	CollisionDamageDealer->OnHarmlessHit.AddDynamic(this, &AInflectionPointProjectile::OnHarmlessHit);

	// instigator is null if the character has already died when the shot is spawned
	if(Instigator == nullptr) {
		return;
	}

	// Avoid collision with instigator
	((ABaseCharacter*)Instigator)->GetCapsuleComponent()->IgnoreActorWhenMoving(this, true);
	CollisionComp->IgnoreActorWhenMoving(Instigator, true);
}

void AInflectionPointProjectile::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit) {
	if(firstHit) {
		DebugLineDrawer->DrawDebugLineTrace(startPos, Hit.Location);
		firstHit = false;
	}
}

void AInflectionPointProjectile::OnDamageHit(float Damage, const FHitResult& Hit) {
	if(CollisionDamageDealer->DestroyOnDamageDealt) {
		MulticastSpawnHitEffect();
	}
	APlayerControllerBase* playerController = Cast<APlayerControllerBase>(Instigator->GetController());
	if(playerController) {
		playerController->DamageDealt();
	}
}

void AInflectionPointProjectile::OnHarmlessHit(const FHitResult& Hit) {
	if(CollisionDamageDealer->DestroyOnHarmlessHit) {
		MulticastSpawnHitEffect();
	}
}

void AInflectionPointProjectile::MulticastSpawnHitEffect_Implementation() {
	if(HitEffectClass == NULL)
		return;

	//Set Spawn Collision Handling Override
	FActorSpawnParameters ActorSpawnParams;
	ActorSpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	ActorSpawnParams.Instigator = Instigator;
	ActorSpawnParams.Owner = GetOwner();

	// spawn the projectile at the muzzle
	GetWorld()->SpawnActor<AActor>(HitEffectClass, GetActorTransform(), ActorSpawnParams);
}