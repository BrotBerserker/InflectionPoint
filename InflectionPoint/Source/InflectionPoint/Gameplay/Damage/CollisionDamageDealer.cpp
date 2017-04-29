
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

	if(!CollisionShapeComponent) {
		CollisionShapeComponent = GetOwner()->FindComponentByClass<UShapeComponent>();
	}

	if(!AssertNotNull(CollisionShapeComponent, GetWorld(), __FILE__, __LINE__, "No UShapeComponent for DamageCollision detection found!"))
		return;
	CollisionShapeComponent->OnComponentHit.AddDynamic(this, &UCollisionDamageDealer::OnHit);
}

void UCollisionDamageDealer::OnHit(class UPrimitiveComponent* HitComp, class AActor* OtherActor, class UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit) {
	bool damageDealed = false;
	if(OtherActor && (!DealDamageOnlyOnCharacters || OtherActor->IsA(ACharacter::StaticClass()))) {
		float damage = InflictDamage(OtherActor);
		damageDealed = damage>0;
		if(damageDealed)
			OnDamageHit.Broadcast(damage, NormalImpulse, Hit);
	} else {
		OnHarmlessHit.Broadcast( NormalImpulse, Hit);
	}
	PerformHitConsequences(damageDealed);
}

float UCollisionDamageDealer::InflictDamage(AActor* DamagedActor) {
	AController* instigator = GetOwner()->Instigator ? GetOwner()->Instigator->GetController() : nullptr;
	return UGameplayStatics::ApplyDamage(DamagedActor,Damage, instigator, GetOwner(), DamageType);
}

void UCollisionDamageDealer::PerformHitConsequences(bool damageDealed) {
	bool needsToBeDestroyed =
		damageDealed && DestroyOnDamageDealed
		|| !damageDealed && DestroyOnHitOnly;
	if(needsToBeDestroyed)
		DestroyOwner();
}

void UCollisionDamageDealer::DestroyOwner() {
	// TODO: destroy over network
	UE_LOG(LogTemp, Warning, TEXT("Destroy"));
	GetOwner()->SetLifeSpan(DestroyDelay + 0.0000001); // 0 dose not destroy o0
}
