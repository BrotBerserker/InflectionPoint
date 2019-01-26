// Fill out your copyright notice in the Description page of Project Settings.

#include "InflectionPoint.h"
#include "LaserWeapon.h"

void ALaserWeapon::Tick(float DeltaTime) {DebugPrint(__FILE__, __LINE__);
	Super::Tick(DeltaTime);

	if(shouldPlayFireFX) {DebugPrint(__FILE__, __LINE__);
		SpawnLaserFX();
	DebugPrint(__FILE__, __LINE__);} else {DebugPrint(__FILE__, __LINE__);
		if(fpLaserTrail)
			fpLaserTrail->DestroyComponent();
		if(tpLaserTrail)
			tpLaserTrail->DestroyComponent();
		fpLaserTrail = nullptr;
		tpLaserTrail = nullptr;
	DebugPrint(__FILE__, __LINE__);}
DebugPrint(__FILE__, __LINE__);}

void ALaserWeapon::MulticastSpawnInstantWeaponFX_Implementation(const FHitResult hitResult) {DebugPrint(__FILE__, __LINE__);
	// dont spawn fx here but in tick
DebugPrint(__FILE__, __LINE__);}

void ALaserWeapon::SpawnLaserFX() {DebugPrint(__FILE__, __LINE__);
	float spread = (OwningCharacter && OwningCharacter->IsAiming) ? AimSpread : Spread;
	const float ConeHalfAngle = FMath::DegreesToRadians(spread * 0.5f);

	const FVector direction = OwningCharacter->FirstPersonCameraComponent->GetForwardVector();
	const FVector StartTrace = OwningCharacter->FirstPersonCameraComponent->GetComponentLocation();
	const FVector ShootDir = WeaponRandomStream.VRandCone(direction, ConeHalfAngle, ConeHalfAngle);
	const FVector EndTrace = StartTrace + ShootDir * Range;
	FHitResult hitResult = AInstantWeapon::WeaponTrace(StartTrace, EndTrace);

	SpawnTrailFX(hitResult);
	SpawnImpactFX(hitResult);
DebugPrint(__FILE__, __LINE__);}

void ALaserWeapon::SpawnTrailFX(const FHitResult hitResult) {DebugPrint(__FILE__, __LINE__);
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
DebugPrint(__FILE__, __LINE__);}

void ALaserWeapon::UpdateTrailFX(UParticleSystemComponent* trail, const FVector& start, const FVector& end) {DebugPrint(__FILE__, __LINE__);
	if(!trail)
		return;
	trail->SetBeamSourcePoint(0, start, 0);
	trail->SetBeamTargetPoint(0, end, 0);
	trail->SetVectorParameter(TrailSourceParamName, start);
	trail->SetVectorParameter(TrailTargetParamName, end);
DebugPrint(__FILE__, __LINE__);}