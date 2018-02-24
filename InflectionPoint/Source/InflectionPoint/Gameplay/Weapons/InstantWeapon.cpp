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

	const FHitResult Impact = WeaponTrace(StartTrace, EndTrace);
	MulticastSpawnWeaponEffects(GetProjectileSpawnLocation(), Impact.Actor == NULL ? EndTrace : Impact.ImpactPoint);
	if(Impact.Actor.IsValid())
		DealDamage(Impact, ShootDir);



}

FHitResult AInstantWeapon::WeaponTrace(const FVector& startTrace, const FVector& endTrace) {
	static FName WeaponFireTag = FName(TEXT("WeaponTrace"));

	// Perform trace to retrieve hit info
	FCollisionQueryParams TraceParams(WeaponFireTag, true, Instigator);
	TraceParams.bTraceAsyncScene = true;
	TraceParams.bReturnPhysicalMaterial = true;

	FHitResult Hit(ForceInit);
	GetWorld()->LineTraceSingleByChannel(Hit, startTrace, endTrace, ECC_GameTraceChannel1, TraceParams);

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

void AInstantWeapon::MulticastSpawnWeaponEffects_Implementation(const FVector& startPoint, const FVector& endPoint) {
	SpawnMuzzleFX();
	SpawnTrailFX(startPoint, endPoint);
	SpawnImpactFX();
}

void AInstantWeapon::SpawnMuzzleFX() {
	if(!MuzzleFX)
		return;

	UParticleSystemComponent* mesh1pFX = UGameplayStatics::SpawnEmitterAttached(MuzzleFX, Mesh1P, NAME_None);
	mesh1pFX->SetWorldLocation(GetProjectileSpawnLocation());
	mesh1pFX->SetWorldRotation(GetProjectileSpawnRotation());
	mesh1pFX->bOwnerNoSee = false;
	mesh1pFX->bOnlyOwnerSee = true;

	//UParticleSystemComponent* mesh3pFX = UGameplayStatics::SpawnEmitterAttached(MuzzleFX, Mesh1P, NAME_None);
	//mesh3pFX->SetWorldLocation(GetProjectileSpawnLocation());
	//mesh3pFX->SetWorldRotation(GetProjectileSpawnRotation());
	//mesh3pFX->bOwnerNoSee = false;
	//mesh3pFX->bOnlyOwnerSee = true;

	if(IsMuzzleFXLooped) {
		StartTimer(this, GetWorld(), "DecativateParticleSystem", 0.1, false, mesh1pFX);
	}

}

void AInstantWeapon::SpawnTrailFX(const FVector& startPoint, const FVector& endPoint) {
	if(!TrailFX)
		return;
	UParticleSystemComponent* TrailPSC = UGameplayStatics::SpawnEmitterAtLocation(this, TrailFX, startPoint);
	if(TrailPSC) {
		TrailPSC->SetVectorParameter(TrailTargetParam, endPoint);
	}
}

void AInstantWeapon::SpawnImpactFX() {

}

void AInstantWeapon::DecativateParticleSystem(UParticleSystemComponent* effect) {
	effect->DeactivateSystem();
}