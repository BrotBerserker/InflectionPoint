// Fill out your copyright notice in the Description page of Project Settings.

#include "InflectionPoint.h"
#include "InstantWeapon.h"



void AInstantWeapon::ExecuteFire() {
	const int32 RandomSeed = FMath::Rand();
	FRandomStream WeaponRandomStream(RandomSeed);

	const float CurrentSpread = 0;
	const float ConeHalfAngle = FMath::DegreesToRadians(CurrentSpread * 0.5f);

	const FVector direction = OwningCharacter->FirstPersonCameraComponent->GetForwardVector();//GetProjectileSpawnRotation().Vector();
	const FVector StartTrace = OwningCharacter->FirstPersonCameraComponent->GetComponentLocation();
	const FVector ShootDir = WeaponRandomStream.VRandCone(direction, ConeHalfAngle, ConeHalfAngle);
	const FVector EndTrace = StartTrace + ShootDir * Range;

	const FHitResult hitResult = WeaponTrace(StartTrace, EndTrace);
	MulticastSpawnWeaponEffects(hitResult);
	if(hitResult.Actor.IsValid())
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
	FPointDamageEvent PointDmg;
	PointDmg.DamageTypeClass = DamageType;
	PointDmg.HitInfo = Impact;
	PointDmg.ShotDirection = ShootDir;
	PointDmg.Damage = Damage;
	
	Impact.GetActor()->TakeDamage(PointDmg.Damage, PointDmg, OwningCharacter->Controller, this);
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
	mesh1pFX->SetWorldLocation(GetFPMuzzleLocation());
	mesh1pFX->SetWorldRotation(GetAimDirection());
	mesh1pFX->bOwnerNoSee = false;
	mesh1pFX->bOnlyOwnerSee = true;

	UParticleSystemComponent* mesh3pFX = UGameplayStatics::SpawnEmitterAttached(MuzzleFX, Mesh3P, NAME_None);
	mesh3pFX->SetWorldLocation(GetTPMuzzleLocation());
	mesh3pFX->SetWorldRotation(GetAimDirection());
	mesh3pFX->bOwnerNoSee = true;
	mesh3pFX->bOnlyOwnerSee = false;

	if(MuzzleFXDuration > 0) {
		StartTimer(this, GetWorld(), "DecativateParticleSystem", MuzzleFXDuration, false, mesh1pFX);
		StartTimer(this, GetWorld(), "DecativateParticleSystem", MuzzleFXDuration, false, mesh3pFX);
	}

}

void AInstantWeapon::SpawnTrailFX(const FVector& endPoint) {
	if(!TrailFX)
		return;
	UParticleSystemComponent* tpTrail = UGameplayStatics::SpawnEmitterAttached(TrailFX, Mesh3P, NAME_None);
	tpTrail->SetWorldLocation(GetTPMuzzleLocation());
	tpTrail->SetWorldRotation(GetAimDirection());
	tpTrail->bOwnerNoSee = true;
	tpTrail->bOnlyOwnerSee = false;

	UParticleSystemComponent* fpTrail = UGameplayStatics::SpawnEmitterAttached(TrailFX, Mesh1P, NAME_None);
	fpTrail->SetWorldLocation(GetFPMuzzleLocation());
	fpTrail->SetWorldRotation(GetAimDirection());
	fpTrail->bOwnerNoSee = false;
	fpTrail->bOnlyOwnerSee = true;

	if(tpTrail && fpTrail) {
		tpTrail->SetVectorParameter(TrailTargetParam, endPoint);
		fpTrail->SetVectorParameter(TrailTargetParam, endPoint);
	}
}

void AInstantWeapon::SpawnImpactFX(FHitResult hitResult) {
	if(!hitResult.bBlockingHit)
		return;
	UParticleSystemComponent* tpTrail = UGameplayStatics::SpawnEmitterAtLocation(this, ImpactFX, hitResult.ImpactPoint);
	tpTrail->SetWorldRotation(hitResult.ImpactNormal.ToOrientationRotator());
}

void AInstantWeapon::DecativateParticleSystem(UParticleSystemComponent* effect) {
	effect->DeactivateSystem();
}