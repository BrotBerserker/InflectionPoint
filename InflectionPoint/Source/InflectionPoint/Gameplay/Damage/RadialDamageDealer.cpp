// Fill out your copyright notice in the Description page of Project Settings.

#include "InflectionPoint.h"
#include "DebugTools/InflectionPointCheatManager.h"
#include "DamageTypes/DefaultDamageType.h"
#include "RadialDamageDealer.h"

// Sets default values for this component's properties
URadialDamageDealer::URadialDamageDealer() {DebugPrint(__FILE__, __LINE__);
DebugPrint(__FILE__, __LINE__);}


// Called when the game starts
void URadialDamageDealer::BeginPlay() {DebugPrint(__FILE__, __LINE__);
	Super::BeginPlay();

	if(!DamageCauser)
		DamageCauser = GetOwner();

	if(DealDamageOnBeginPlay)
		DealDamage();
DebugPrint(__FILE__, __LINE__);}


void URadialDamageDealer::DealDamage() {DebugPrint(__FILE__, __LINE__);
	if(!GetOwner()->HasAuthority())
		return;

	auto controller = GetOwner()->GetInstigatorController();
	auto location = GetOwner()->GetActorLocation();
	StartTimer(this, GetWorld(), "ExecuteDealDamage", 0.001f + DamageDealDelay, false, location, controller);
DebugPrint(__FILE__, __LINE__);}


void URadialDamageDealer::ExecuteDealDamage(FVector location, AController* controller) {DebugPrint(__FILE__, __LINE__);
	DrawDebugSpheres(location);

	// Apply the Damage
	auto damagedActors = ApplyRadialDamageWithFalloff(location, controller);

	// Show hitmarker if a character was hit
	auto playerController = Cast<APlayerControllerBase>(controller);
	for(int i = 0; i < damagedActors.Num() && playerController; ++i) {DebugPrint(__FILE__, __LINE__);
		if(damagedActors[i]->IsA(ABaseCharacter::StaticClass())) {DebugPrint(__FILE__, __LINE__);
			playerController->DamageDealt();
			return; // show damageDealt only once
		DebugPrint(__FILE__, __LINE__);}
	DebugPrint(__FILE__, __LINE__);}
DebugPrint(__FILE__, __LINE__);}

void URadialDamageDealer::DrawDebugSpheres(FVector &location) {DebugPrint(__FILE__, __LINE__);
	auto cheatManager = Cast<UInflectionPointCheatManager>(GetWorld()->GetFirstPlayerController()->CheatManager);
	if(cheatManager && cheatManager->IsDebugProjectileLineTraceEnabled) {DebugPrint(__FILE__, __LINE__);
		DrawDebugSphere(GetWorld(), location, DamageOuterRadius, 16, FColor(71, 244, 65), true, 1000);
		DrawDebugSphere(GetWorld(), location, DamageInnerRadius, 16, FColor(244, 95, 66), true, 1000);
	DebugPrint(__FILE__, __LINE__);}
DebugPrint(__FILE__, __LINE__);}

void URadialDamageDealer::DrawDebugTraceLine(FVector &start, FVector &end, FColor color) {DebugPrint(__FILE__, __LINE__);
	auto cheatManager = Cast<UInflectionPointCheatManager>(GetWorld()->GetFirstPlayerController()->CheatManager);
	if(cheatManager && cheatManager->IsDebugProjectileLineTraceEnabled) {DebugPrint(__FILE__, __LINE__);
		DrawDebugLine(GetWorld(), start, end, color, true, 1000);
	DebugPrint(__FILE__, __LINE__);}
DebugPrint(__FILE__, __LINE__);}


TMap<AActor*, TArray<UPrimitiveComponent*>> URadialDamageDealer::GetAllActorComponentsInRange(const FVector& origin, float radius) {DebugPrint(__FILE__, __LINE__);
	FCollisionQueryParams SphereParams(FName("ApplyRadialDamage"), false);

	// query scene to see what we hit
	TArray<FOverlapResult> overlaps;
	UWorld* World = GEngine->GetWorldFromContextObjectChecked(GetWorld());
	World->OverlapMultiByObjectType(overlaps, origin, FQuat::Identity, FCollisionObjectQueryParams(FCollisionObjectQueryParams::InitType::AllObjects), FCollisionShape::MakeSphere(radius), SphereParams);
	
	// Convert to Actor Array
	TMap<AActor*, TArray<UPrimitiveComponent*>> overlapHits;
	for(int32 i = 0; i < overlaps.Num(); ++i) {DebugPrint(__FILE__, __LINE__);
		if(overlaps[i].GetActor() && overlaps[i].GetActor()->bCanBeDamaged) {DebugPrint(__FILE__, __LINE__);
			TArray<UPrimitiveComponent*>& HitList = overlapHits.FindOrAdd(overlaps[i].GetActor());
			HitList.Add(overlaps[i].Component.Get());
		DebugPrint(__FILE__, __LINE__);}
	DebugPrint(__FILE__, __LINE__);}	
	return overlapHits;
DebugPrint(__FILE__, __LINE__);}

TMap<AActor*, TArray<FHitResult>> URadialDamageDealer::GetHitableActorsInRange(const FVector& origin) {DebugPrint(__FILE__, __LINE__);
	TMap<AActor*, TArray<UPrimitiveComponent*>> actorComponentsInRange = GetAllActorComponentsInRange(origin, DamageOuterRadius);

	// See if Components of the Actor that are in range can be hit
	TMap<AActor*, TArray<FHitResult>> damagableActors;
	for(TMap<AActor*, TArray<UPrimitiveComponent*> >::TIterator It(actorComponentsInRange); It; ++It) {DebugPrint(__FILE__, __LINE__);
		AActor* const victim = It.Key();
		TArray<UPrimitiveComponent*> const& components = It.Value();

		for(int32 i = 0; i < components.Num(); ++i) {DebugPrint(__FILE__, __LINE__);
			auto component = components[i];
			auto hitResult = RaycastToComponent(component, origin);
			if(WasRaycastBlocked(component, hitResult)) {DebugPrint(__FILE__, __LINE__);
				TArray<FHitResult>& HitList = damagableActors.FindOrAdd(victim);
				HitList.Add(hitResult);
			DebugPrint(__FILE__, __LINE__);}
		DebugPrint(__FILE__, __LINE__);}
	DebugPrint(__FILE__, __LINE__);}
	return damagableActors;
DebugPrint(__FILE__, __LINE__);}

TArray<AActor*> URadialDamageDealer::ApplyRadialDamageWithFalloff(const FVector& origin, AController* instigatedByController) {DebugPrint(__FILE__, __LINE__);
	auto damagableActors = GetHitableActorsInRange(origin);
	return DealDamage(origin, damagableActors, instigatedByController);
DebugPrint(__FILE__, __LINE__);}

TArray<AActor*> URadialDamageDealer::DealDamage(const FVector & origin, TMap<AActor*, TArray<FHitResult>>& damagableActors, AController * instigatedByController) {DebugPrint(__FILE__, __LINE__);
	TArray<AActor*> DamagedActorList = TArray<AActor*>();

	FRadialDamageEvent DmgEvent;
	DmgEvent.DamageTypeClass = DamageTypeClass ? DamageTypeClass : (TSubclassOf<UDamageType>)UDefaultDamageType::StaticClass();
	DmgEvent.Origin = origin;
	DmgEvent.Params = FRadialDamageParams(BaseDamage, MinimumDamage, DamageInnerRadius, DamageOuterRadius, Falloff);

	// call damage function on each affected actors
	for(TMap<AActor*, TArray<FHitResult>>::TIterator It(damagableActors); It; ++It) {DebugPrint(__FILE__, __LINE__);
		AActor* const victim = It.Key();
		TArray<FHitResult> const& hitResults = It.Value();

		DmgEvent.ComponentHits = hitResults;
		float damage = victim->TakeDamage(BaseDamage, DmgEvent, instigatedByController, DamageCauser);
		if(damage > 0)
			DamagedActorList.Add(victim);
	DebugPrint(__FILE__, __LINE__);}
	return DamagedActorList;
DebugPrint(__FILE__, __LINE__);}


FHitResult URadialDamageDealer::RaycastToComponent(UPrimitiveComponent* victimComp, FVector const& origin) {DebugPrint(__FILE__, __LINE__);
	FCollisionQueryParams lineParams(FName("ComponentIsVisibleFrom"), true);
	FVector traceEnd = victimComp->Bounds.Origin;
	FVector traceStart = origin;

	// tiny nudge so LineTraceSingle doesn't early out with no hits
	if(traceEnd == traceStart)
		traceStart.Z += 0.01f;

	FHitResult hitResult;
	bool traceBlocked = GetWorld()->LineTraceSingleByChannel(hitResult, traceStart, traceEnd, (ECollisionChannel)DamagePreventionChannel, lineParams);
	if(!traceBlocked) {DebugPrint(__FILE__, __LINE__);
		// didn't hit anything, assume nothing blocking the damage (creating a fake hit result)
		FVector const fakeHitLoc = victimComp->GetComponentLocation();
		FVector const fakeHitNorm = (origin - fakeHitLoc).GetSafeNormal();
		hitResult = FHitResult(victimComp->GetOwner(), victimComp, fakeHitLoc, fakeHitNorm);
	DebugPrint(__FILE__, __LINE__);}
	DrawDebugTraceLine(traceStart, traceEnd, WasRaycastBlocked(victimComp, hitResult) ? FColor::Green : FColor::Red);
	return hitResult;
DebugPrint(__FILE__, __LINE__);}

bool URadialDamageDealer::WasRaycastBlocked(UPrimitiveComponent* victimComp, FHitResult& hitResult) {DebugPrint(__FILE__, __LINE__);	
	return !hitResult.bBlockingHit || hitResult.Component == victimComp;
DebugPrint(__FILE__, __LINE__);}
