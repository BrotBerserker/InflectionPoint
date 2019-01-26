
// Fill out your copyright notice in the Description page of Project Settings.

#include "InflectionPoint.h"
#include "MortalityProvider.h"
#include "CollisionDamageDealer.h"


// Sets default values for this component's properties
UCollisionDamageDealer::UCollisionDamageDealer() {DebugPrint(__FILE__, __LINE__);
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = false;
DebugPrint(__FILE__, __LINE__);}


// Called when the game starts
void UCollisionDamageDealer::BeginPlay() {DebugPrint(__FILE__, __LINE__);
	Super::BeginPlay();

	if(!collisionShapeComponent) {DebugPrint(__FILE__, __LINE__);
		collisionShapeComponent = GetOwner()->FindComponentByClass<UShapeComponent>();
	DebugPrint(__FILE__, __LINE__);}

	if(!AssertNotNull(collisionShapeComponent, GetWorld(), __FILE__, __LINE__, "No UShapeComponent for DamageCollision detection found!"))
		return;

	if(!AssertNotNull(DamageType, GetWorld(), __FILE__, __LINE__))
		return;
	if(!DamageCauser)
		DamageCauser = GetOwner();
	collisionShapeComponent->OnComponentHit.AddDynamic(this, &UCollisionDamageDealer::OnCollision);
	collisionShapeComponent->OnComponentBeginOverlap.AddDynamic(this, &UCollisionDamageDealer::OnOverlap);
DebugPrint(__FILE__, __LINE__);}

void UCollisionDamageDealer::OnCollision(class UPrimitiveComponent* HitComp, class AActor* OtherActor, class UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit) {DebugPrint(__FILE__, __LINE__);
	OnOtherHit.Broadcast(Hit);
	if(DestroyOnOtherHit) {DebugPrint(__FILE__, __LINE__);
		DestroyOwner();
	DebugPrint(__FILE__, __LINE__);}
DebugPrint(__FILE__, __LINE__);}

void UCollisionDamageDealer::OnOverlap(class UPrimitiveComponent* HitComp, class AActor* OtherActor, class UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult & SweepResult) {DebugPrint(__FILE__, __LINE__);
	float damage = InflictDamage(OtherActor);
	if(OtherComp == TargetComponent) {DebugPrint(__FILE__, __LINE__);
		OnTargetHit.Broadcast(damage, OtherActor, OtherComp);
		if(DestroyOnTargetHit) {DebugPrint(__FILE__, __LINE__);
			DestroyOwner();
		DebugPrint(__FILE__, __LINE__);}
	DebugPrint(__FILE__, __LINE__);} else {DebugPrint(__FILE__, __LINE__);
		OnPawnHit.Broadcast(damage, SweepResult);
		if(DestroyOnPawnHit) {DebugPrint(__FILE__, __LINE__);
			DestroyOwner();
		DebugPrint(__FILE__, __LINE__);}
	DebugPrint(__FILE__, __LINE__);}
DebugPrint(__FILE__, __LINE__);}

float UCollisionDamageDealer::InflictDamage(AActor* DamagedActor) {DebugPrint(__FILE__, __LINE__);
	if(!GetOwner()->HasAuthority())
		return 0;
	AController* instigator = GetOwner()->Instigator ? GetOwner()->Instigator->GetController() : nullptr;
	return UGameplayStatics::ApplyDamage(DamagedActor, Damage, instigator, DamageCauser, DamageType);
DebugPrint(__FILE__, __LINE__);}

void UCollisionDamageDealer::DestroyOwner() {DebugPrint(__FILE__, __LINE__);
	GetOwner()->SetActorHiddenInGame(true);

	if(DestroyDelay == 0) {DebugPrint(__FILE__, __LINE__);
		collisionShapeComponent->OnComponentBeginOverlap.RemoveDynamic(this, &UCollisionDamageDealer::OnOverlap); // avoid unwanted hits
		collisionShapeComponent->OnComponentHit.RemoveDynamic(this, &UCollisionDamageDealer::OnCollision);
		GetOwner()->Destroy();
		return;
	DebugPrint(__FILE__, __LINE__);}

	GetOwner()->SetLifeSpan(DestroyDelay + 0.0000001); // 0 does not destroy o0

DebugPrint(__FILE__, __LINE__);}
