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

	if(DealDamageOnBeginPlay)
		DealDamage();
}


void URadialDamageDealer::DealDamage() {
	if(!GetOwner()->HasAuthority())
		return;
	auto controller = Cast<APlayerControllerBase>(GetOwner()->GetInstigatorController());
	auto instigator = GetOwner()->Instigator;
	auto location = GetOwner()->GetActorLocation();
	StartTimer(this, GetWorld(), "ExecuteDealDamage", 0.001f + DamageDealDelay, false, location, controller, instigator);
}


void URadialDamageDealer::ExecuteDealDamage(FVector location, APlayerControllerBase* controller, AActor* instigator) {
	auto actorList = ApplyRadialDamageWithFalloff(location, instigator, controller);
	
	// Draw debugs
	auto cheatManager = Cast<UInflectionPointCheatManager>(GetWorld()->GetFirstPlayerController()->CheatManager);
	if(cheatManager && cheatManager->IsDebugProjectileLineTraceEnabled) {
		DrawDebugSphere(GetWorld(), location, DamageOuterRadius, 16, FColor(71, 244, 65), true, 1000);
		DrawDebugSphere(GetWorld(), location, DamageInnerRadius, 16, FColor(244, 95, 66), true, 1000);
	}

	// Show hitmarker
	for(int i = 0; i < actorList.Num() && controller; ++i) {
		if(actorList[i]->IsA(ABaseCharacter::StaticClass())) {
			controller->DamageDealt();
			return; // show damageDealt only once
		}
	}
}


TArray<AActor*> URadialDamageDealer::ApplyRadialDamageWithFalloff(const FVector& Origin, AActor* DamageCauser, AController* InstigatedByController) {
	static FName NAME_ApplyRadialDamage = FName(TEXT("ApplyRadialDamage"));
	FCollisionQueryParams SphereParams(NAME_ApplyRadialDamage, false);//, DamageCauser);

	//SphereParams.AddIgnoredActors(IgnoreActors);

	// query scene to see what we hit
	TArray<FOverlapResult> Overlaps;
	UWorld* World = GEngine->GetWorldFromContextObject(GetWorld());
	World->OverlapMultiByObjectType(Overlaps, Origin, FQuat::Identity, FCollisionObjectQueryParams(FCollisionObjectQueryParams::InitType::AllDynamicObjects), FCollisionShape::MakeSphere(DamageOuterRadius), SphereParams);

	// collate into per-actor list of hit components
	TMap<AActor*, TArray<FHitResult> > OverlapComponentMap;
	for(int32 Idx = 0; Idx<Overlaps.Num(); ++Idx) {
		FOverlapResult const& Overlap = Overlaps[Idx];
		AActor* const OverlapActor = Overlap.GetActor();

		if(OverlapActor &&
			OverlapActor->bCanBeDamaged &&
			(OverlapActor != DamageCauser || DealDamageToDamageCauser) &&
			Overlap.Component.IsValid()) {
			UE_LOG(LogTemp, Warning, TEXT("DealDamage"));

			FHitResult Hit;
			if(DamagePreventionChannel == ECC_MAX || CanHitComponent(Overlap.Component.Get(), Origin, (ECollisionChannel)DamagePreventionChannel, Hit)) {
				TArray<FHitResult>& HitList = OverlapComponentMap.FindOrAdd(OverlapActor);
				HitList.Add(Hit);
			}
		}
	}

	bool bAppliedDamage = false;
	TArray<AActor*> DamagedActorList = TArray<AActor*>();

	if(OverlapComponentMap.Num() > 0) {
		// make sure we have a good damage type
		TSubclassOf<UDamageType> const ValidDamageTypeClass = DamageTypeClass ? DamageTypeClass : TSubclassOf<UDamageType>(UDamageType::StaticClass());

		FRadialDamageEvent DmgEvent;
		DmgEvent.DamageTypeClass = ValidDamageTypeClass;
		DmgEvent.Origin = Origin;
		DmgEvent.Params = FRadialDamageParams(BaseDamage, MinimumDamage, DamageInnerRadius, DamageOuterRadius, Falloff);

		// call damage function on each affected actors
		for(TMap<AActor*, TArray<FHitResult> >::TIterator It(OverlapComponentMap); It; ++It) {
			AActor* const Victim = It.Key();
			TArray<FHitResult> const& ComponentHits = It.Value();
			DmgEvent.ComponentHits = ComponentHits;

			float damage = Victim->TakeDamage(BaseDamage, DmgEvent, InstigatedByController, DamageCauser);
			if(damage > 0)
				DamagedActorList.Add(Victim);
		}
	}

	return DamagedActorList;
}

/** @RETURN True if weapon trace from Origin hits component VictimComp.  OutHitResult will contain properties of the hit. */
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