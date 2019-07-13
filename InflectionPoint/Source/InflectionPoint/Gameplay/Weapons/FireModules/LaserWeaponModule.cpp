// Fill out your copyright notice in the Description page of Project Settings.

#include "InflectionPoint.h"
#include "LaserWeaponModule.h"


void ULaserWeaponModule::TickComponent(float DeltaTime, enum ELevelTick tickType, FActorComponentTickFunction *thisTickFunction) {
	Super::TickComponent(DeltaTime, tickType, thisTickFunction);

	if(shouldPlayFireFX) {
		SpawnLaserFX();
	} else {
		DisposeLaserTrailFX();
	}
}

void ULaserWeaponModule::OnDeactivate() {
	Super::OnDeactivate();
	DisposeLaserTrailFX();
}

void ULaserWeaponModule::DisposeLaserTrailFX() {
	if(fpLaserTrail)
		fpLaserTrail->DestroyComponent();
	if(tpLaserTrail)
		tpLaserTrail->DestroyComponent();
	fpLaserTrail = nullptr;
	tpLaserTrail = nullptr;
}

void ULaserWeaponModule::SpawnInstantWeaponFX(const FHitResult hitResult) {
	// dont spawn fx here but in tick
}

void ULaserWeaponModule::SpawnLaserFX() {
	FHitResult hitResult = UInstantWeaponModule::WeaponTraceShootDirection(false);
	SpawnLaserTrailFX(hitResult);
	SpawnImpactFX(hitResult);
}

void ULaserWeaponModule::SpawnLaserTrailFX(const FHitResult hitResult) {
	if(!TrailFX)
		return;
	auto endPoint = hitResult.bBlockingHit ? hitResult.ImpactPoint : hitResult.TraceEnd;

	if(!fpLaserTrail)
		fpLaserTrail = Weapon->SpawnTrailFX(TrailFX, endPoint, TrailSourceParamName, TrailTargetParamName, true);
	if(!tpLaserTrail)
		tpLaserTrail = Weapon->SpawnTrailFX(TrailFX, endPoint, TrailSourceParamName, TrailTargetParamName, false);
	if(fpLaserTrail)
		UpdateLaserTrailFX(fpLaserTrail, Weapon->GetFPMuzzleLocation(), endPoint);
	if(tpLaserTrail)
		UpdateLaserTrailFX(tpLaserTrail, Weapon->GetTPMuzzleLocation(), endPoint);
}

void ULaserWeaponModule::UpdateLaserTrailFX(UParticleSystemComponent* trail, const FVector& start, const FVector& end) {
	if(!trail)
		return;
	trail->SetBeamSourcePoint(0, start, 0);
	trail->SetBeamTargetPoint(0, end, 0);
	trail->SetVectorParameter(TrailSourceParamName, start);
	trail->SetVectorParameter(TrailTargetParamName, end);
}