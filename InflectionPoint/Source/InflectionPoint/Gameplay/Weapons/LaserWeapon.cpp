// Fill out your copyright notice in the Description page of Project Settings.

#include "InflectionPoint.h"
#include "LaserWeapon.h"

void ALaserWeapon::Tick(float DeltaTime) {
	Super::Tick(DeltaTime);

	if(shouldPlayFireFX) {
		SpawnLaserFX();
	} else {
		if(fpLaserTrail)
			fpLaserTrail->DestroyComponent();
		if(tpLaserTrail)
			tpLaserTrail->DestroyComponent();
		fpLaserTrail = nullptr;
		tpLaserTrail = nullptr;
	}
}

void ALaserWeapon::MulticastSpawnInstantWeaponFX_Implementation(const FHitResult hitResult) {
	// dont spawn fx here but in tick
}

void ALaserWeapon::SpawnLaserFX() {
	FHitResult hitResult = AInstantWeapon::WeaponTraceShootDirection(false);
	SpawnTrailFX(hitResult);
	SpawnImpactFX(hitResult);
}

void ALaserWeapon::SpawnTrailFX(const FHitResult hitResult) {
	if(!TrailFX)
		return;
	auto endPoint = hitResult.bBlockingHit ? hitResult.ImpactPoint : hitResult.TraceEnd;

	if(!fpLaserTrail)
		fpLaserTrail = AInstantWeapon::SpawnTrailFX(endPoint, true);
	if(!tpLaserTrail)
		tpLaserTrail = AInstantWeapon::SpawnTrailFX(endPoint, false);
	if(fpLaserTrail)
		UpdateTrailFX(fpLaserTrail, GetFPMuzzleLocation(), endPoint);
	if(tpLaserTrail)
		UpdateTrailFX(tpLaserTrail, GetTPMuzzleLocation(), endPoint);
}

void ALaserWeapon::UpdateTrailFX(UParticleSystemComponent* trail, const FVector& start, const FVector& end) {
	if(!trail)
		return;
	trail->SetBeamSourcePoint(0, start, 0);
	trail->SetBeamTargetPoint(0, end, 0);
	trail->SetVectorParameter(TrailSourceParamName, start);
	trail->SetVectorParameter(TrailTargetParamName, end);
}