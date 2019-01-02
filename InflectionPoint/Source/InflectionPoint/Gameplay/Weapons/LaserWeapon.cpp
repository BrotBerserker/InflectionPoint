// Fill out your copyright notice in the Description page of Project Settings.

#include "InflectionPoint.h"
#include "LaserWeapon.h"

void ALaserWeapon::Tick(float DeltaTime) {
	Super::Tick(DeltaTime);

	if(GetGameTimeSinceCreation() > FireInterval && timeSinceLastShot <= FireInterval) {
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
	float spread = (OwningCharacter && OwningCharacter->IsAiming) ? AimSpread : Spread;
	const float ConeHalfAngle = FMath::DegreesToRadians(spread * 0.5f);

	const FVector direction = OwningCharacter->FirstPersonCameraComponent->GetForwardVector();
	const FVector StartTrace = OwningCharacter->FirstPersonCameraComponent->GetComponentLocation();
	const FVector ShootDir = WeaponRandomStream.VRandCone(direction, ConeHalfAngle, ConeHalfAngle);
	const FVector EndTrace = StartTrace + ShootDir * Range;
	FHitResult hitResult = AInstantWeapon::WeaponTrace(StartTrace, EndTrace);

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