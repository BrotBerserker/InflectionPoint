// Copyright 1998-2016 Epic Games, Inc. All Rights Reserved.

#include "InflectionPoint.h"
#include "InflectionPointProjectile.h"
#include "Gameplay/Characters/BaseCharacter.h"
#include "Gameplay/Controllers/PlayerControllerBase.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "ProjectileWeapon.h"

void AInflectionPointProjectile::GetLifetimeReplicatedProps(TArray< FLifetimeProperty > & OutLifetimeProps) const {
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AInflectionPointProjectile, HomingTarget);
}

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

	// Initialize MortalityProvider
	MortalityProvider = CreateDefaultSubobject<UMortalityProvider>(TEXT("MortalityProvider"));
	MortalityProvider->SetIsReplicated(true);

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

	// Die after 5 seconds by default
	InitialLifeSpan = 20.0f;
}

void AInflectionPointProjectile::BeginPlay() {
	Super::BeginPlay();

	startPos = GetActorLocation();

	// We can't do this in the constructor because the Events are not yet initialized then
	CollisionDamageDealer->OnTargetHit.AddDynamic(this, &AInflectionPointProjectile::OnTargetHit);
	CollisionDamageDealer->OnPawnHit.AddDynamic(this, &AInflectionPointProjectile::OnPawnHit);
	CollisionDamageDealer->OnOtherHit.AddDynamic(this, &AInflectionPointProjectile::OnOtherHit);
	MortalityProvider->OnDeath.AddDynamic(this, &AInflectionPointProjectile::DestroyProjectile);
	MortalityProvider->StartHealth = 1;

	// Set the Weapon as DamageDealer
	CollisionDamageDealer->DamageCauser = GetOwner();

	// instigator is null if the character has already died when the shot is spawned
	if(Instigator == nullptr) {
		return;
	}

	// Avoid collision with instigator
	((ABaseCharacter*)Instigator)->GetCapsuleComponent()->IgnoreActorWhenMoving(this, true);
	CollisionComp->IgnoreActorWhenMoving(Instigator, true);
}

void AInflectionPointProjectile::OnRep_HomingTarget() {
	SetHomingTarget(HomingTarget);
}

void AInflectionPointProjectile::SetHomingTarget(UPrimitiveComponent* Target){
	HomingTarget = Target;
	ProjectileMovement->HomingTargetComponent = HomingTarget;
	CollisionDamageDealer->TargetComponent = HomingTarget;
}

void AInflectionPointProjectile::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit) {
	if(firstHit) {
		DebugLineDrawer->DrawDebugLineTrace(startPos, Hit.Location);
		firstHit = false;
	}
}

void AInflectionPointProjectile::OnTargetHit(float Damage, AActor* OtherActor, UPrimitiveComponent* OtherComp) {
	if(CollisionDamageDealer->DestroyOnTargetHit) {
		MulticastSpawnHitEffect();
	}
}

void AInflectionPointProjectile::OnPawnHit(float Damage, const FHitResult& Hit) {
	if(CollisionDamageDealer->DestroyOnPawnHit) {
		MulticastSpawnHitEffect();
	}
	APlayerControllerBase* playerController = Cast<APlayerControllerBase>(Instigator->GetController());
	if(playerController && Damage > 0) {
		playerController->DamageDealt(false);
	}
}

void AInflectionPointProjectile::OnOtherHit(const FHitResult& Hit) {
	if(CollisionDamageDealer->DestroyOnOtherHit) {
		MulticastSpawnHitEffect();
	}
}

void AInflectionPointProjectile::MulticastSpawnHitEffect_Implementation() {
	if(HitEffectClass == NULL)
		return;

	// avoid projectile getting destroyed by explosion
	bCanBeDamaged = false;

	//Set Spawn Collision Handling Override
	FActorSpawnParameters ActorSpawnParams;
	ActorSpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	ActorSpawnParams.Instigator = Instigator;
	ActorSpawnParams.Owner = GetOwner();

	// spawn the projectile at the muzzle
	GetWorld()->SpawnActor<AActor>(HitEffectClass, GetActorTransform(), ActorSpawnParams);
}

void AInflectionPointProjectile::DestroyProjectile(AController* KillingPlayer, AActor* DamageCauser) {
	MulticastSpawnHitEffect();
	SetLifeSpan(0.0000001);
}