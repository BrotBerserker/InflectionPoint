// Copyright 1998-2016 Epic Games, Inc. All Rights Reserved.

#include "InflectionPoint.h"
#include "InflectionPointProjectile.h"
#include "Gameplay/Characters/BaseCharacter.h"
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

	// Set as root component
	RootComponent = CollisionComp;

	// Use a ProjectileMovementComponent to govern this projectile's movement
	ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileComp"));
	ProjectileMovement->UpdatedComponent = CollisionComp;
	ProjectileMovement->InitialSpeed = 3000.f;
	ProjectileMovement->MaxSpeed = 3000.f;
	ProjectileMovement->bRotationFollowsVelocity = true;
	ProjectileMovement->bShouldBounce = true;

	// Die after 3 seconds by default
	InitialLifeSpan = 3.0f;
}

void AInflectionPointProjectile::BeginPlay() {
	Super::BeginPlay();
	APawn* instigator = GetInstigator();
	// instigator is null if the character has already died when the shot is spawned
	if(instigator == nullptr) {
		return;
	}

	((ABaseCharacter*)instigator)->GetCapsuleComponent()->IgnoreActorWhenMoving(this, true);

	CollisionComp->IgnoreActorWhenMoving(instigator, true);
}

void AInflectionPointProjectile::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit) {
	//UE_LOG(LogTemp, Warning, TEXT("hit mit folgendem shit: %s"), *OtherActor->GetName());
	//UE_LOG(LogTemp, Warning, TEXT("hit mit folgendem componentshit: %s"), *OtherComp->GetName());

	//// Only add impulse and destroy projectile if we hit a physics
	//if((OtherActor != NULL) && (OtherActor != this) && (OtherComp != NULL) && OtherComp->IsSimulatingPhysics()) {
	//	OtherComp->AddImpulseAtLocation(GetVelocity() * 100.0f, GetActorLocation());
	//	Destroy();
	//}
}