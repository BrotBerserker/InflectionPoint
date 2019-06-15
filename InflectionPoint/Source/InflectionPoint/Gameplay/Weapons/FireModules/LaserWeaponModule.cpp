// Fill out your copyright notice in the Description page of Project Settings.

#include "InflectionPoint.h"
#include "LaserWeaponModule.h"


void ULaserWeaponModule::Tick(float DeltaTime) {
	Super::Tick(DeltaTime);

	if(false/*shouldPlayFireFX*/) {
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

void ULaserWeaponModule::MulticastSpawnInstantWeaponFX_Implementation(const FHitResult hitResult) {
	// dont spawn fx here but in tick
}

void ULaserWeaponModule::SpawnLaserFX() {
	FHitResult hitResult = UInstantWeaponModule::WeaponTraceShootDirection(false);
	SpawnTrailFX(hitResult);
	SpawnImpactFX(hitResult);
}

void ULaserWeaponModule::SpawnTrailFX(const FHitResult hitResult) {
	if(!TrailFX)
		return;
	auto endPoint = hitResult.bBlockingHit ? hitResult.ImpactPoint : hitResult.TraceEnd;

	if(!fpLaserTrail)
		fpLaserTrail = UInstantWeaponModule::SpawnTrailFX(endPoint, true);
	if(!tpLaserTrail)
		tpLaserTrail = UInstantWeaponModule::SpawnTrailFX(endPoint, false);
	if(fpLaserTrail)
		UpdateTrailFX(fpLaserTrail, Weapon->GetFPMuzzleLocation(), endPoint);
	if(tpLaserTrail)
		UpdateTrailFX(tpLaserTrail, Weapon->GetTPMuzzleLocation(), endPoint);
}

void ULaserWeaponModule::UpdateTrailFX(UParticleSystemComponent* trail, const FVector& start, const FVector& end) {
	if(!trail)
		return;
	trail->SetBeamSourcePoint(0, start, 0);
	trail->SetBeamTargetPoint(0, end, 0);
	trail->SetVectorParameter(TrailSourceParamName, start);
	trail->SetVectorParameter(TrailTargetParamName, end);
}