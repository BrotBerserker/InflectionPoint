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
	DrawDebugs(location);

	// Apply the Damage
	auto actorList = ApplyRadialDamageWithFalloff(location, controller);

	// Show hitmarker if a character was hit
	for(int i = 0; i < actorList.Num() && controller; ++i) {
		if(actorList[i]->IsA(ABaseCharacter::StaticClass())) {
			controller->DamageDealt();
			return; // show damageDealt only once
		}
	}
}

void URadialDamageDealer::DrawDebugs(FVector &location) {
	auto cheatManager = Cast<UInflectionPointCheatManager>(GetWorld()->GetFirstPlayerController()->CheatManager);
	if(cheatManager && cheatManager->IsDebugProjectileLineTraceEnabled) {
		DrawDebugSphere(GetWorld(), location, DamageOuterRadius, 16, FColor(71, 244, 65), true, 1000);
		DrawDebugSphere(GetWorld(), location, DamageInnerRadius, 16, FColor(244, 95, 66), true, 1000);
	}
}


TMap<AActor*, TArray<FHitResult> > URadialDamageDealer::GetActorsInRange(const FVector& origin) {
	static FName NAME_ApplyRadialDamage = FName(TEXT("ApplyRadialDamage"));
	FCollisionQueryParams SphereParams(NAME_ApplyRadialDamage, false);

	// query scene to see what we hit
	TArray<FOverlapResult> Overlaps;
	UWorld* World = GEngine->GetWorldFromContextObject(GetWorld());
	World->OverlapMultiByObjectType(Overlaps, origin, FQuat::Identity, FCollisionObjectQueryParams(FCollisionObjectQueryParams::InitType::AllDynamicObjects), FCollisionShape::MakeSphere(DamageOuterRadius), SphereParams);
	
	// collate into per-actor list of hit components
	TMap<AActor*, TArray<FHitResult> > OverlapComponentMap;
	for(int32 i = 0; i<Overlaps.Num(); ++i) {
		FOverlapResult const& Overlap = Overlaps[i];
		AActor* const OverlapActor = Overlap.GetActor();

		if(OverlapActor && OverlapActor->bCanBeDamaged && Overlap.Component.IsValid()) {
			FHitResult Hit;
			if(CanHitComponent(Overlap.Component.Get(), origin, (ECollisionChannel)DamagePreventionChannel, Hit)) {
				TArray<FHitResult>& HitList = OverlapComponentMap.FindOrAdd(OverlapActor);
				HitList.Add(Hit);
			}
		}
	}
	return OverlapComponentMap;
}

TArray<AActor*> URadialDamageDealer::ApplyRadialDamageWithFalloff(const FVector& origin, AController* instigatedByController) {
	auto overlappingActors = GetActorsInRange(origin);

	if(overlappingActors.Num() < 1)
		return TArray<AActor*>();
	
	return DealDamage(origin, overlappingActors, instigatedByController);
}

TArray<AActor*> URadialDamageDealer::DealDamage(const FVector & origin, TMap<AActor *, TArray<FHitResult>> &OverlapComponentMap, AController * instigatedByController) {
	TArray<AActor*> DamagedActorList = TArray<AActor*>();
	
	FRadialDamageEvent DmgEvent;
	DmgEvent.DamageTypeClass = DamageTypeClass ? DamageTypeClass : UDamageType::StaticClass();
	DmgEvent.Origin = origin;
	DmgEvent.Params = FRadialDamageParams(BaseDamage, MinimumDamage, DamageInnerRadius, DamageOuterRadius, Falloff);

	// call damage function on each affected actors
	for(TMap<AActor*, TArray<FHitResult> >::TIterator It(OverlapComponentMap); It; ++It) {
		AActor* const Victim = It.Key();
		TArray<FHitResult> const& ComponentHits = It.Value();
		DmgEvent.ComponentHits = ComponentHits;

		float damage = Victim->TakeDamage(BaseDamage, DmgEvent, instigatedByController, DamageCauser);
		if(damage > 0)
			DamagedActorList.Add(Victim);
	}
	return DamagedActorList;
}


bool URadialDamageDealer::CanHitComponent(UPrimitiveComponent* VictimComp, FVector const& Origin, ECollisionChannel TraceChannel, FHitResult& OutHitResult) {
	static FName NAME_ComponentIsVisibleFrom = FName(TEXT("ComponentIsVisibleFrom"));
	//FCollisionQueryParams LineParams(NAME_ComponentIsVisibleFrom, true, IgnoredActor);
	FCollisionQueryParams LineParams(NAME_ComponentIsVisibleFrom, true);

	// Do a trace from origin to middle of box
	UWorld* const World = VictimComp->GetWorld();
	check(World);

	FVector const TraceEnd = VictimComp->Bounds.Origin;
	FVector TraceStart = Origin;
	if(Origin == TraceEnd) {
		// tiny nudge so LineTraceSingle doesn't early out with no hits
		TraceStart.Z += 0.01f;
	}
	bool const bHadBlockingHit = World->LineTraceSingleByChannel(OutHitResult, TraceStart, TraceEnd, TraceChannel, LineParams);
	//::DrawDebugLine(World, TraceStart, TraceEnd, FLinearColor::Red, true);

	// If there was a blocking hit, it will be the last one
	if(bHadBlockingHit) {
		if(OutHitResult.Component == VictimComp) {
			// if blocking hit was the victim component, it is visible
			return true;
		} else {
			// if we hit something else blocking, it's not
			UE_LOG(LogDamage, Log, TEXT("Radial Damage to %s blocked by %s (%s)"), *GetNameSafe(VictimComp), *GetNameSafe(OutHitResult.GetActor()), *GetNameSafe(OutHitResult.Component.Get()));
			return false;
		}
	}

	// didn't hit anything, assume nothing blocking the damage and victim is consequently visible
	// but since we don't have a hit result to pass back, construct a simple one, modeling the damage as having hit a point at the component's center.
	FVector const FakeHitLoc = VictimComp->GetComponentLocation();
	FVector const FakeHitNorm = (Origin - FakeHitLoc).GetSafeNormal();		// normal points back toward the epicenter
	OutHitResult = FHitResult(VictimComp->GetOwner(), VictimComp, FakeHitLoc, FakeHitNorm);
	return true;
}