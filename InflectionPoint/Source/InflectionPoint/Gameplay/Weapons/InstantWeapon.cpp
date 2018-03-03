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

	const FHitResult hitResult = WeaponTrace(StartTrace, EndTrace);
	MulticastSpawnWeaponEffects(hitResult);
	DrawDebugLineTrace(hitResult.ImpactPoint);
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

void AInstantWeapon::DealDamage(const FHitResult& Impact, const FVector& ShootDir) {
	if(!(Impact.Actor.IsValid() && Impact.Actor.Get()->IsA(ABaseCharacter::StaticClass())))
		return;
	FPointDamageEvent PointDmg;
	PointDmg.DamageTypeClass = DamageType;
	PointDmg.HitInfo = Impact;
	PointDmg.ShotDirection = ShootDir;
	PointDmg.Damage = Damage;

	Impact.GetActor()->TakeDamage(PointDmg.Damage, PointDmg, OwningCharacter->Controller, this);

	// notify controller
	auto controller = Cast<APlayerControllerBase>(OwningCharacter->Controller);
	if(controller)
		controller->DamageDealt();
}

void AInstantWeapon::MulticastSpawnWeaponEffects_Implementation(FHitResult hitResult) {
	SpawnMuzzleFX();
	SpawnTrailFX(hitResult.bBlockingHit ? hitResult.ImpactPoint : hitResult.TraceEnd);
	SpawnImpactFX(hitResult);
}

void AInstantWeapon::SpawnMuzzleFX() {
	if(!MuzzleFX)
		return;

	UParticleSystemComponent* mesh1pFX = UGameplayStatics::SpawnEmitterAttached(MuzzleFX, Mesh1P, NAME_None);
	if(mesh1pFX) {
		mesh1pFX->SetWorldLocation(GetFPMuzzleLocation());
		mesh1pFX->SetWorldRotation(GetAimDirection());
		mesh1pFX->bOwnerNoSee = false;
		mesh1pFX->bOnlyOwnerSee = true;
		if(MuzzleFXDuration > 0)
			StartTimer(this, GetWorld(), "DecativateParticleSystem", MuzzleFXDuration, false, mesh1pFX);
	}
	UParticleSystemComponent* mesh3pFX = UGameplayStatics::SpawnEmitterAttached(MuzzleFX, Mesh3P, NAME_None);
	if(mesh3pFX) {
		mesh3pFX->SetWorldLocation(GetTPMuzzleLocation());
		mesh3pFX->SetWorldRotation(GetAimDirection());
		mesh3pFX->bOwnerNoSee = true;
		mesh3pFX->bOnlyOwnerSee = false;
		if(MuzzleFXDuration > 0)
			StartTimer(this, GetWorld(), "DecativateParticleSystem", MuzzleFXDuration, false, mesh3pFX);
	}
}

void AInstantWeapon::SpawnTrailFX(const FVector& endPoint) {
	if(!TrailFX)
		return;
	UParticleSystemComponent* tpTrail = UGameplayStatics::SpawnEmitterAttached(TrailFX, Mesh3P, NAME_None);
	if(tpTrail) {
		tpTrail->SetWorldLocation(GetTPMuzzleLocation());
		tpTrail->SetWorldRotation(GetAimDirection());
		tpTrail->bOwnerNoSee = true;
		tpTrail->bOnlyOwnerSee = false;
		tpTrail->SetVectorParameter(TrailTargetParamName, endPoint);
	}

	UParticleSystemComponent* fpTrail = UGameplayStatics::SpawnEmitterAttached(TrailFX, Mesh1P, NAME_None);
	if(fpTrail) {
		fpTrail->SetWorldLocation(GetFPMuzzleLocation());
		fpTrail->SetWorldRotation(GetAimDirection());
		fpTrail->bOwnerNoSee = false;
		fpTrail->bOnlyOwnerSee = true;
		fpTrail->SetVectorParameter(TrailTargetParamName, endPoint);
	}
}

void AInstantWeapon::SpawnImpactFX(FHitResult hitResult) {
	if(!hitResult.bBlockingHit)
		return;
	UParticleSystemComponent* tpTrail = UGameplayStatics::SpawnEmitterAtLocation(this, ImpactFX, hitResult.ImpactPoint);
	if(tpTrail)
		tpTrail->SetWorldRotation(hitResult.ImpactNormal.ToOrientationRotator());
}

void AInstantWeapon::DecativateParticleSystem(UParticleSystemComponent* effect) {
	effect->DeactivateSystem();
}

void AInstantWeapon::DrawDebugLineTrace(const FVector& endPoint) {
	auto cheatManager = Cast<UInflectionPointCheatManager>(GetWorld()->GetFirstPlayerController()->CheatManager);
	if(!(cheatManager && cheatManager->IsDebugProjectileLineTraceEnabled))
		return;
	if(Recorder) {
		DrawDebugLine(GetWorld(), GetFPMuzzleLocation(), endPoint, PlayerDebugColor, true, -1, 0, 0.5);
		DrawDebugPoint(GetWorld(), endPoint, 3, PlayerDebugColor, true);
	} else {
		DrawDebugLine(GetWorld(), GetFPMuzzleLocation(), endPoint, ReplayDebugColor, true, -1, 0, 0.5);
		DrawDebugPoint(GetWorld(), endPoint, 3, ReplayDebugColor, true);
	}
}

void AInstantWeapon::OnEquip() {
	Super::OnEquip();
	// always start with a new RandomStream for replay Precision
	WeaponRandomStream = FRandomStream(0);
}