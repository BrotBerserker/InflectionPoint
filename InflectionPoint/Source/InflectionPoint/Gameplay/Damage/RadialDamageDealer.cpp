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


TMap<AActor*, TArray<UPrimitiveComponent*>> URadialDamageDealer::GetAllActorComponentsInRange(const FVector& origin, float radius) {
	FCollisionQueryParams SphereParams(FName("ApplyRadialDamage"), false);

	// query scene to see what we hit
	TArray<FOverlapResult> overlaps;
	UWorld* World = GEngine->GetWorldFromContextObject(GetWorld());
	World->OverlapMultiByObjectType(overlaps, origin, FQuat::Identity, FCollisionObjectQueryParams(FCollisionObjectQueryParams::InitType::AllObjects), FCollisionShape::MakeSphere(radius), SphereParams);
	
	// Convert to Actor Array
	TMap<AActor*, TArray<UPrimitiveComponent*>> overlapHits;
	for(int32 i = 0; i < overlaps.Num(); ++i) {
		if(overlaps[i].GetActor() && overlaps[i].GetActor()->bCanBeDamaged) {
			TArray<UPrimitiveComponent*>& HitList = overlapHits.FindOrAdd(overlaps[i].GetActor());
			HitList.Add(overlaps[i].Component.Get());
		}
	}	
	return overlapHits;
}

TMap<AActor*, TArray<FHitResult>> URadialDamageDealer::GetHitableActorsInRange(const FVector& origin) {
	TMap<AActor*, TArray<UPrimitiveComponent*>> actorComponentsInRange = GetAllActorComponentsInRange(origin, DamageOuterRadius);

	// See if Components of the Actor that are in range can be hit
	TMap<AActor*, TArray<FHitResult>> damagableActors;
	for(TMap<AActor*, TArray<UPrimitiveComponent*> >::TIterator It(actorComponentsInRange); It; ++It) {
		AActor* const victim = It.Key();
		TArray<UPrimitiveComponent*> const& components = It.Value();

		for(int32 i = 0; i < components.Num(); ++i) {
			auto component = components[i];
			auto hitResult = RaycastToComponent(component, origin);
			if(WasRaycastBlocked(component, hitResult)) {
				TArray<FHitResult>& HitList = damagableActors.FindOrAdd(victim);
				HitList.Add(hitResult);
			}
		}
	}
	return damagableActors;
}

TArray<AActor*> URadialDamageDealer::ApplyRadialDamageWithFalloff(const FVector& origin, AController* instigatedByController) {
	auto damagableActors = GetHitableActorsInRange(origin);
	return DealDamage(origin, damagableActors, instigatedByController);
}

TArray<AActor*> URadialDamageDealer::DealDamage(const FVector & origin, TMap<AActor*, TArray<FHitResult>>& damagableActors, AController * instigatedByController) {
	TArray<AActor*> DamagedActorList = TArray<AActor*>();

	FRadialDamageEvent DmgEvent;
	DmgEvent.DamageTypeClass = DamageTypeClass ? DamageTypeClass : UDamageType::StaticClass();
	DmgEvent.Origin = origin;
	DmgEvent.Params = FRadialDamageParams(BaseDamage, MinimumDamage, DamageInnerRadius, DamageOuterRadius, Falloff);

	// call damage function on each affected actors
	for(TMap<AActor*, TArray<FHitResult> >::TIterator It(damagableActors); It; ++It) {
		AActor* const victim = It.Key();
		TArray<FHitResult> const& hitResults = It.Value();

		DmgEvent.ComponentHits = hitResults;
		float damage = victim->TakeDamage(BaseDamage, DmgEvent, instigatedByController, DamageCauser);
		if(damage > 0)
			DamagedActorList.Add(victim);
	}
	return DamagedActorList;
}


FHitResult URadialDamageDealer::RaycastToComponent(UPrimitiveComponent* victimComp, FVector const& origin) {
	FCollisionQueryParams lineParams(FName("ComponentIsVisibleFrom"), true);
	FVector traceEnd = victimComp->Bounds.Origin;
	FVector traceStart = origin;

	// tiny nudge so LineTraceSingle doesn't early out with no hits
	if(traceEnd == traceStart)
		traceStart.Z += 0.01f;

	FHitResult hitResult;
	bool traceBlocked = GetWorld()->LineTraceSingleByChannel(hitResult, traceStart, traceEnd, (ECollisionChannel)DamagePreventionChannel, lineParams);
	if(!traceBlocked) {
		// didn't hit anything, assume nothing blocking the damage (creating a fake hit result)
		FVector const fakeHitLoc = victimComp->GetComponentLocation();
		FVector const fakeHitNorm = (origin - fakeHitLoc).GetSafeNormal();
		hitResult = FHitResult(victimComp->GetOwner(), victimComp, fakeHitLoc, fakeHitNorm);
	}
	DrawDebugTraceLine(traceStart, traceEnd, WasRaycastBlocked(victimComp, hitResult) ? FColor::Green : FColor::Red);
	return hitResult;
}

bool URadialDamageDealer::WasRaycastBlocked(UPrimitiveComponent* victimComp, FHitResult& hitResult) {	
	return !hitResult.bBlockingHit || hitResult.Component == victimComp;
}