
// Fill out your copyright notice in the Description page of Project Settings.

#include "InflectionPoint.h"
#include "MortalityProvider.h"
#include "CollisionDamageDealer.h"


// Sets default values for this component's properties
UCollisionDamageDealer::UCollisionDamageDealer() {
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = false;
}


// Called when the game starts
void UCollisionDamageDealer::BeginPlay() {
	Super::BeginPlay();

	if(!collisionShapeComponent) {
		collisionShapeComponent = GetOwner()->FindComponentByClass<UShapeComponent>();
	}

	if(!AssertNotNull(collisionShapeComponent, GetWorld(), __FILE__, __LINE__, "No UShapeComponent for DamageCollision detection found!"))
		return;

	if(!AssertNotNull(DamageType, GetWorld(), __FILE__, __LINE__))
		return;

	//collisionShapeComponent->OnComponentHit.AddDynamic(this, &UCollisionDamageDealer::OnHit);
	collisionShapeComponent->OnComponentBeginOverlap.AddDynamic(this, &UCollisionDamageDealer::OnHit);
}

void UCollisionDamageDealer::OnHit(class UPrimitiveComponent* HitComp, class AActor* OtherActor, class UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult & SweepResult) {
	bool damageDealt = false;
	if(OtherActor && (!DealDamageOnlyOnCharacters || OtherActor->IsA(ACharacter::StaticClass()))) {
		// projectile doesn't deal damage on client side, so damage will be 0 for clients
		float damage = InflictDamage(OtherActor);
		damageDealt = damage > 0; 
		if(damageDealt)
			OnDamageHit.Broadcast(damage, SweepResult);
	} else {
		OnHarmlessHit.Broadcast(SweepResult);
	}
	PerformHitConsequences(damageDealt);
}

float UCollisionDamageDealer::InflictDamage(AActor* DamagedActor) {
	AController* instigator = GetOwner()->Instigator ? GetOwner()->Instigator->GetController() : nullptr;
	return UGameplayStatics::ApplyDamage(DamagedActor, Damage, instigator, GetOwner(), DamageType);
}

void UCollisionDamageDealer::PerformHitConsequences(bool damageDealt) {
	bool needsToBeDestroyed =
		(damageDealt && DestroyOnDamageDealt)
		|| (!damageDealt && DestroyOnHarmlessHit);
	if(needsToBeDestroyed)
		DestroyOwner();
}

void UCollisionDamageDealer::DestroyOwner() {
	GetOwner()->SetActorHiddenInGame(true);

	if(DestroyDelay == 0) {
		collisionShapeComponent->OnComponentBeginOverlap.RemoveDynamic(this, &UCollisionDamageDealer::OnHit); // avoid unwanted hits
		GetOwner()->Destroy();
		return;
	}

	GetOwner()->SetLifeSpan(DestroyDelay + 0.0000001); // 0 does not destroy o0
	
}
