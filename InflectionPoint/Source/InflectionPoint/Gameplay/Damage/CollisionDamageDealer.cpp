
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

	//ProjectileMovement = GetOwner()->FindComponentByClass<UProjectileMovementComponent>();
	//ProjectileMovement->OnProjectileBounce.AddDynamic(this, &UCollisionDamageDealer::OnBounce);
	if(!CollisionShapeComponent) {
		CollisionShapeComponent = GetOwner()->FindComponentByClass<UShapeComponent>();
	}

	if(!AssertNotNull(CollisionShapeComponent, GetWorld(), __FILE__, __LINE__, "No UShapeComponent for DamageCollision detection found!"))
		return;
	CollisionShapeComponent->OnComponentHit.AddDynamic(this, &UCollisionDamageDealer::OnHit);
}



// Called every frame
void UCollisionDamageDealer::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) {
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	// ...
}

void UCollisionDamageDealer::OnHit(class UPrimitiveComponent* HitComp, class AActor* OtherActor, class UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit) {
	bool damageDealed = false;
	if(OtherActor && (!DealDamageOnlyOnCharacters || OtherActor->IsA(ACharacter::StaticClass()))) {
		damageDealed = InflictDamage(OtherActor);
	}
	PerformHitConsequences(damageDealed, Hit);
}

bool UCollisionDamageDealer::InflictDamage(AActor* DamagedActor) {
	//AssertTrue(DamageType != NULL, GetWorld(), __FILE__, __LINE__, "No DamageType selected");
	//AssertNotNull(DamageEventInstigator, GetWorld(), __FILE__, __LINE__,"No DamageEventInstigator set");
	float dealedDamage = UGameplayStatics::ApplyDamage(DamagedActor,Damage, DamageEventInstigator,GetOwner(), DamageType);
	// maybe do something here later on
	return dealedDamage > 0;
}

void UCollisionDamageDealer::PerformHitConsequences(bool damageDealed, const FHitResult & Hit) {
	bool needsToBeDestroyed =
		damageDealed && DestroyOnDamageDealed
		|| !damageDealed && DestroyOnHitOnly;
	auto ClassesToSpawn = damageDealed ? ClassesToSpawnOnDamageDealed : ClassesToSpawnOnHitOnly;

	for(auto* item : ClassesToSpawn) {
		SpawnFromClassOnHit(Hit, item);
	}
	if(needsToBeDestroyed)
		DestroyOwner();
}

void UCollisionDamageDealer::SpawnFromClassOnHit(const FHitResult & Hit, UClass * &item) {
	FRotator Rotation(0.0f, 0.0f, 0.0f);
	FVector Location(Hit.ImpactPoint);
	FActorSpawnParameters SpawnInfo;
	GetWorld()->SpawnActor(item, &Location, &Rotation, SpawnInfo);
}

void UCollisionDamageDealer::DestroyOwner() {
	// TODO: destroy over network
	UE_LOG(LogTemp, Warning, TEXT("Destroy"));
	GetOwner()->SetLifeSpan(DestroyDelay + 0.0000001); // 0 dose not destroy o0
}
