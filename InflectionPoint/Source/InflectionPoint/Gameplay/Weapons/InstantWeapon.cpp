// Fill out your copyright notice in the Description page of Project Settings.

#include "InflectionPoint.h"
#include "DebugTools/InflectionPointCheatManager.h"
#include "Gameplay/Characters/BaseCharacter.h"
#include "Gameplay/Controllers/PlayerControllerBase.h"
#include "InstantWeapon.h"


void AInstantWeapon::ExecuteFire() {
	const float ConeHalfAngle = FMath::DegreesToRadians(Spread * 0.5f);

	const FVector direction = OwningCharacter->FirstPersonCameraComponent->GetForwardVector();
	const FVector StartTrace = OwningCharacter->FirstPersonCameraComponent->GetComponentLocation();
	const FVector ShootDir = WeaponRandomStream.VRandCone(direction, ConeHalfAngle, ConeHalfAngle);
	const FVector EndTrace = StartTrace + ShootDir * Range;

	FHitResult hitResult = WeaponTrace(StartTrace, EndTrace);
	MulticastSpawnInstantWeaponFX(hitResult);
	DrawDebugLineTrace(hitResult);
	DealDamage(hitResult, ShootDir);
}

FHitResult AInstantWeapon::WeaponTrace(const FVector& startTrace, const FVector& endTrace) {
	static FName WeaponFireTag = FName(TEXT("WeaponTrace"));

	// Perform trace to retrieve hit info
	FCollisionQueryParams TraceParams(WeaponFireTag, true, Instigator);
	TraceParams.bTraceAsyncScene = true;
	TraceParams.bReturnPhysicalMaterial = true;

	FHitResult Hit(ForceInit);
	GetWorld()->LineTraceSingleByChannel(Hit, startTrace, endTrace, (ECollisionChannel)CollisionChannel, TraceParams);
	return Hit;
}

void AInstantWeapon::DealDamage(const FHitResult hitResult,const FVector& ShootDir) {
	if(!(hitResult.Actor.IsValid() && hitResult.Actor.Get()->IsA(ABaseCharacter::StaticClass())))
		return;
	FPointDamageEvent PointDmg;
	PointDmg.DamageTypeClass = DamageType;
	PointDmg.HitInfo = hitResult;
	PointDmg.ShotDirection = ShootDir;
	PointDmg.Damage = Damage;

	hitResult.GetActor()->TakeDamage(PointDmg.Damage, PointDmg, OwningCharacter->Controller, this);

	// notify controller
	auto controller = Cast<APlayerControllerBase>(OwningCharacter->Controller);
	if(controller)
		controller->DamageDealt();
}

void AInstantWeapon::MulticastSpawnInstantWeaponFX_Implementation(const FHitResult hitResult) {
	SpawnTrailFX(hitResult);
	SpawnImpactFX(hitResult);
}

void AInstantWeapon::SpawnTrailFX(const FHitResult hitResult) {
	if(!TrailFX)
		return;
	auto endPint = hitResult.bBlockingHit ? hitResult.ImpactPoint : hitResult.TraceEnd;
	UParticleSystemComponent* tpTrail = UGameplayStatics::SpawnEmitterAttached(TrailFX, Mesh3P, NAME_None);
	if(tpTrail) {
		tpTrail->SetWorldLocation(GetTPMuzzleLocation());
		tpTrail->SetWorldRotation(GetAimDirection());
		tpTrail->bOwnerNoSee = true;
		tpTrail->bOnlyOwnerSee = false;
		tpTrail->SetVectorParameter(TrailTargetParamName, endPint);
	}

	UParticleSystemComponent* fpTrail = UGameplayStatics::SpawnEmitterAttached(TrailFX, Mesh1P, NAME_None);
	if(fpTrail) {
		fpTrail->SetWorldLocation(GetFPMuzzleLocation());
		fpTrail->SetWorldRotation(GetAimDirection());
		fpTrail->bOwnerNoSee = false;
		fpTrail->bOnlyOwnerSee = true;
		fpTrail->SetVectorParameter(TrailTargetParamName, endPint);
	}
}

void AInstantWeapon::SpawnImpactFX(const FHitResult hitResult) {
	if(!hitResult.bBlockingHit)
		return;
	UParticleSystemComponent* tpTrail = UGameplayStatics::SpawnEmitterAtLocation(this, ImpactFX, hitResult.ImpactPoint);
	if(tpTrail)
		tpTrail->SetWorldRotation(hitResult.ImpactNormal.ToOrientationRotator());
}

void AInstantWeapon::DrawDebugLineTrace(const FHitResult hitResult) {
	if(!hitResult.bBlockingHit)
		return;
	auto cheatManager = Cast<UInflectionPointCheatManager>(GetWorld()->GetFirstPlayerController()->CheatManager);
	if(!(cheatManager && cheatManager->IsDebugProjectileLineTraceEnabled))
		return;
	if(Recorder) {
		DrawDebugLine(GetWorld(), GetFPMuzzleLocation(), hitResult.ImpactPoint, PlayerDebugColor, true, -1, 0, 0.5);
		DrawDebugPoint(GetWorld(), hitResult.ImpactPoint, 3, PlayerDebugColor, true);
	} else {
		DrawDebugLine(GetWorld(), GetFPMuzzleLocation(), hitResult.ImpactPoint, ReplayDebugColor, true, -1, 0, 0.5);
		DrawDebugPoint(GetWorld(), hitResult.ImpactPoint, 3, ReplayDebugColor, true);
	}
}

void AInstantWeapon::OnEquip() {
	Super::OnEquip();
	// always start with a new RandomStream for replay Precision
	WeaponRandomStream = FRandomStream(0);
}