// Fill out your copyright notice in the Description page of Project Settings.

#include "InflectionPoint.h"
#include "DebugTools/InflectionPointCheatManager.h"
#include "RadialDamageDealer.h"


// Sets default values for this component's properties
URadialDamageDealer::URadialDamageDealer() {
}


// Called when the game starts
void URadialDamageDealer::BeginPlay() {
	Super::BeginPlay();

	if(!DamageCauser)
		DamageCauser = GetOwner();

	if(DealDamageOnBeginPlay)
		DealDamage();
}


void URadialDamageDealer::DealDamage() {
	if(!GetOwner()->HasAuthority())
		return;

	auto controller = Cast<APlayerControllerBase>(GetOwner()->GetInstigatorController());
	auto location = GetOwner()->GetActorLocation();
	StartTimer(this, GetWorld(), "ExecuteDealDamage", 0.001f + DamageDealDelay, false, location, controller);
}


void URadialDamageDealer::ExecuteDealDamage(FVector location, APlayerControllerBase* controller) {
	DrawDebugSpheres(location);

	// Apply the Damage
	auto damagedActors = ApplyRadialDamageWithFalloff(location, controller);

	// Show hitmarker if a character was hit
	for(int i = 0; i < damagedActors.Num() && controller; ++i) {
		if(damagedActors[i]->IsA(ABaseCharacter::StaticClass())) {
			controller->DamageDealt();
			return; // show damageDealt only once
		}
	}
}

void URadialDamageDealer::DrawDebugSpheres(FVector &location) {
	auto cheatManager = Cast<UInflectionPointCheatManager>(GetWorld()->GetFirstPlayerController()->CheatManager);
	if(cheatManager && cheatManager->IsDebugProjectileLineTraceEnabled) {
		DrawDebugSphere(GetWorld(), location, DamageOuterRadius, 16, FColor(71, 244, 65), true, 1000);
		DrawDebugSphere(GetWorld(), location, DamageInnerRadius, 16, FColor(244, 95, 66), true, 1000);
	}
}

void URadialDamageDealer::DrawDebugTraceLine(FVector &start, FVector &end, FColor color) {
	auto cheatManager = Cast<UInflectionPointCheatManager>(GetWorld()->GetFirstPlayerController()->CheatManager);
	if(cheatManager && cheatManager->IsDebugProjectileLineTraceEnabled) {
		DrawDebugLine(GetWorld(), start, end, color, true, 1000);
	}
}


TArray<AActor*> URadialDamageDealer::GetAllActorsInRange(const FVector& origin, float radius) {
	FCollisionQueryParams SphereParams(FName("ApplyRadialDamage"), false);

	// query scene to see what we hit
	TArray<FOverlapResult> overlaps;
	UWorld* World = GEngine->GetWorldFromContextObject(GetWorld());
	World->OverlapMultiByObjectType(overlaps, origin, FQuat::Identity, FCollisionObjectQueryParams(FCollisionObjectQueryParams::InitType::AllObjects), FCollisionShape::MakeSphere(radius), SphereParams);
	
	// Convert to Actor Array
	TArray<AActor*> actors;
	for(int32 i = 0; i < overlaps.Num(); ++i) {
		if(overlaps[i].GetActor() && overlaps[i].GetActor()->bCanBeDamaged)
			actors.Add(overlaps[i].GetActor());
	}	
	return actors;
}

TArray<AActor*> URadialDamageDealer::GetHitableActorsInRange(const FVector& origin) {
	TArray<AActor*> actorsInRange = GetAllActorsInRange(origin, DamageOuterRadius);
	if(!DamageCanBeBlocked)
		return actorsInRange;

	// See if Actors can be hit
	TArray<AActor*> damagableActors;
	for(int32 i = 0; i < actorsInRange.Num(); ++i) {
		auto actorInRange = actorsInRange[i];

		if(CanHitActor(actorInRange, origin)) {
			damagableActors.Add(actorInRange);
		}
	}
	return damagableActors;
}

TArray<AActor*> URadialDamageDealer::ApplyRadialDamageWithFalloff(const FVector& origin, AController* instigatedByController) {
	auto actors = GetHitableActorsInRange(origin);

	if(actors.Num() <= 0)
		return TArray<AActor*>();

	return DealDamage(origin, actors, instigatedByController);
}

TArray<AActor*> URadialDamageDealer::DealDamage(const FVector & origin, TArray<AActor*> &actors, AController * instigatedByController) {
	TArray<AActor*> DamagedActorList = TArray<AActor*>();

	FRadialDamageEvent DmgEvent;
	DmgEvent.DamageTypeClass = DamageTypeClass ? DamageTypeClass : UDamageType::StaticClass();
	DmgEvent.Origin = origin;
	DmgEvent.Params = FRadialDamageParams(BaseDamage, MinimumDamage, DamageInnerRadius, DamageOuterRadius, Falloff);

	// call damage function on each affected actors
	for(int32 i = 0; i < actors.Num(); ++i) {
		auto victim = actors[i];

		float damage = victim->TakeDamage(BaseDamage, DmgEvent, instigatedByController, DamageCauser);
		if(damage > 0)
			DamagedActorList.Add(victim);
	}
	return DamagedActorList;
}


bool URadialDamageDealer::CanHitActor(AActor* actor, FVector const& origin) {
	auto victimComp = (UPrimitiveComponent*)actor->GetRootComponent();
	FCollisionQueryParams lineParams(FName("ComponentIsVisibleFrom"), true);
	FVector traceEnd = victimComp->Bounds.Origin;
	FVector traceStart = origin;

	// tiny nudge so LineTraceSingle doesn't early out with no hits
	if(traceEnd == traceStart) 
		traceStart.Z += 0.01f;
	
	FHitResult hitResult;
	bool traceBlocked = GetWorld()->LineTraceSingleByChannel(hitResult, traceStart, traceEnd, (ECollisionChannel)DamagePreventionChannel, lineParams);
	bool canHitActor = !traceBlocked || hitResult.Component == victimComp || hitResult.Component->GetOwner() == actor;
	DrawDebugTraceLine(traceStart, traceEnd, canHitActor ? FColor::Green : FColor::Red);
	return canHitActor;
}