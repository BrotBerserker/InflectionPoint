// Fill out your copyright notice in the Description page of Project Settings.

#include "InflectionPoint.h"
#include "Gameplay/Weapons/BaseWeapon.h"
#include "InstantWeaponModule.h"

void UInstantWeaponModule::Initialize() {
	Super::Initialize();
	DebugLineDrawer = GetOwner()->FindComponentByClass<UDebugLineDrawer>();
	// always start with a new RandomStream for replay Precision
	WeaponRandomStream = FRandomStream(0);
}

void UInstantWeaponModule::OnActivate() {
	Super::OnActivate();
}

void UInstantWeaponModule::PreExecuteFire() {
	damageWasDealt = false;
	hitWasHeadshot = false;
}

void UInstantWeaponModule::ExecuteFire() {
	FHitResult hitResult = WeaponTraceShootDirection(true);
	SpawnInstantWeaponFX(hitResult);

	if(DebugLineDrawer && hitResult.bBlockingHit) {
		DebugLineDrawer->DrawDebugLineTrace(Weapon->GetFPMuzzleLocation(), hitResult.ImpactPoint);
	}
	DealDamage(hitResult, OwningCharacter->FirstPersonCameraComponent->GetForwardVector());
}

void UInstantWeaponModule::PostExecuteFire() {
	auto controller = Cast<APlayerControllerBase>(OwningCharacter->Controller);
	if(controller && damageWasDealt)
		controller->DamageDealt(hitWasHeadshot);
}

FHitResult UInstantWeaponModule::WeaponTraceShootDirection(bool applySpread) {
	float spread = (OwningCharacter && OwningCharacter->IsAiming) ? AimSpread : Spread;
	const float ConeHalfAngle = FMath::DegreesToRadians(spread * 0.5f);
	FVector ShootDir = OwningCharacter->FirstPersonCameraComponent->GetForwardVector();
	if(applySpread)
		ShootDir = WeaponRandomStream.VRandCone(ShootDir, ConeHalfAngle, ConeHalfAngle);
	const FVector StartTrace = OwningCharacter->FirstPersonCameraComponent->GetComponentLocation();
	const FVector EndTrace = StartTrace + ShootDir * Range;
	if(ShootRadius <= 0)
		return WeaponTrace(StartTrace, EndTrace);
	return WeaponBoxTrace(StartTrace, EndTrace, ShootRadius);
}

FHitResult UInstantWeaponModule::WeaponTrace(const FVector& startTrace, const FVector& endTrace) {
	static FName WeaponFireTag = FName(TEXT("WeaponTrace"));

	// Perform trace to retrieve hit info
	FCollisionQueryParams TraceParams(WeaponFireTag, true, Weapon->Instigator);
	TraceParams.bReturnPhysicalMaterial = true;

	FHitResult Hit(ForceInit);
	GetWorld()->LineTraceSingleByChannel(Hit, startTrace, endTrace, (ECollisionChannel)CollisionChannel, TraceParams);
	return Hit;
}

FHitResult UInstantWeaponModule::WeaponBoxTrace(const FVector& startTrace, const FVector& endTrace, int radius) {
	FCollisionShape Shape = FCollisionShape::MakeBox(FVector(radius * 2, radius * 2, 50));
	FQuat ShapeRotation = FQuat(0, 0, 0, 0);
	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(OwningCharacter);
	QueryParams.AddIgnoredActor(Weapon);
	FHitResult hitResult;
	GetWorld()->SweepSingleByChannel(hitResult, startTrace, endTrace, ShapeRotation, (ECollisionChannel)CollisionChannel, Shape, QueryParams);
	return hitResult;
}

void UInstantWeaponModule::DealDamage(const FHitResult hitResult, const FVector& ShootDir) {
	if(!hitResult.Actor.IsValid())
		return;

	// to notify a controller if a character was damaged
	if(hitResult.Actor.Get()->IsA(ABaseCharacter::StaticClass())) {
		hitWasHeadshot = hitWasHeadshot || (HeadshotBonusDamage != 0 && Cast<ABaseCharacter>(hitResult.Actor.Get())->IsHitAHeadshot(hitResult));
		damageWasDealt = true;
	}

	FPointDamageEvent PointDmg;
	PointDmg.DamageTypeClass = DamageType;
	PointDmg.HitInfo = hitResult;
	PointDmg.ShotDirection = ShootDir;
	PointDmg.Damage = Damage + (hitWasHeadshot ? HeadshotBonusDamage : 0);

	hitResult.GetActor()->TakeDamage(PointDmg.Damage, PointDmg, OwningCharacter->Controller, Weapon);
}

void UInstantWeaponModule::SpawnInstantWeaponFX(const FHitResult hitResult) {
	SpawnTrailFX(hitResult);
	SpawnImpactFX(hitResult);
}

void UInstantWeaponModule::SpawnTrailFX(const FHitResult hitResult) {
	if(!TrailFX)
		return;
	auto endPoint = hitResult.bBlockingHit ? hitResult.ImpactPoint : hitResult.TraceEnd;
	Weapon->MulticastSpawnTrailFX(TrailFX, endPoint, TrailSourceParamName, TrailTargetParamName, true);
	Weapon->MulticastSpawnTrailFX(TrailFX, endPoint, TrailSourceParamName, TrailTargetParamName, false);
}

void UInstantWeaponModule::SpawnImpactFX(const FHitResult hitResult) {
	if(!hitResult.bBlockingHit)
		return;
	Weapon->MulticastSpawnFXAtLocation(ImpactFX, hitResult.ImpactPoint, hitResult.ImpactNormal.ToOrientationRotator(), ImpactFXScale);
}
