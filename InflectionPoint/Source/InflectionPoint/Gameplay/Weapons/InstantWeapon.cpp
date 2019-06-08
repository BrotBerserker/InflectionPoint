// Fill out your copyright notice in the Description page of Project Settings.

#include "InflectionPoint.h"
#include "DebugTools/InflectionPointCheatManager.h"
#include "Gameplay/Characters/BaseCharacter.h"
#include "Gameplay/Controllers/PlayerControllerBase.h"
#include "Gameplay/Damage/MortalityProvider.h"
#include "InstantWeapon.h"


AInstantWeapon::AInstantWeapon() {
	DebugLineDrawer = CreateDefaultSubobject<UDebugLineDrawer>(TEXT("DebugLineDrawer"));

	// set default values
	AISuitabilityWeaponRangeCurve.GetRichCurve()->AddKey(Range >= 100 ? Range - 100 : 1, 1);
	AISuitabilityWeaponRangeCurve.GetRichCurve()->AddKey(Range >= 100 ? Range : 100, 0);
}

void AInstantWeapon::PreExecuteFire() {
	damageWasDealt = false;
	hitWasHeadshot = false;
}

void AInstantWeapon::ExecuteFire() {
	FHitResult hitResult = WeaponTraceShootDirection(true);
	MulticastSpawnInstantWeaponFX(hitResult);

	if(hitResult.bBlockingHit) {
		DebugLineDrawer->DrawDebugLineTrace(GetFPMuzzleLocation(), hitResult.ImpactPoint);
	}
	DealDamage(hitResult, OwningCharacter->FirstPersonCameraComponent->GetForwardVector());
}

void AInstantWeapon::PostExecuteFire() {
	auto controller = Cast<APlayerControllerBase>(OwningCharacter->Controller);
	if(controller && damageWasDealt)
		controller->DamageDealt(hitWasHeadshot);
}

FHitResult AInstantWeapon::WeaponTraceShootDirection(bool applySpread) {
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

FHitResult AInstantWeapon::WeaponBoxTrace(const FVector& startTrace, const FVector& endTrace, int radius) {
	FCollisionShape Shape = FCollisionShape::MakeBox(FVector(radius * 2, radius * 2, 50));
	FQuat ShapeRotation = FQuat(0, 0, 0, 0);
	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(GetOwner());
	QueryParams.AddIgnoredActor(this);
	FHitResult hitResult;
	GetWorld()->SweepSingleByChannel(hitResult, startTrace, endTrace, ShapeRotation, (ECollisionChannel)CollisionChannel, Shape, QueryParams);
	return hitResult;
}

void AInstantWeapon::DealDamage(const FHitResult hitResult, const FVector& ShootDir) {
	if(!hitResult.Actor.IsValid())
		return;

	// to notify a controller if a character was damaged
	if(hitResult.Actor.Get()->IsA(ABaseCharacter::StaticClass())) {
		hitWasHeadshot = hitWasHeadshot || (HeadshotBonusDamage != 0 && Cast<ABaseCharacter>(hitResult.Actor.Get())->IsHitAHeadshot(hitResult));
	}

	FPointDamageEvent PointDmg;
	PointDmg.DamageTypeClass = DamageType;
	PointDmg.HitInfo = hitResult;
	PointDmg.ShotDirection = ShootDir;
	PointDmg.Damage = Damage + (hitWasHeadshot ? HeadshotBonusDamage : 0);

	float dealtDamage = hitResult.GetActor()->TakeDamage(PointDmg.Damage, PointDmg, OwningCharacter->Controller, this);
	if(dealtDamage > 0 && hitResult.GetActor()->FindComponentByClass<UMortalityProvider>()) {
		damageWasDealt = true;
	}
}

void AInstantWeapon::MulticastSpawnInstantWeaponFX_Implementation(const FHitResult hitResult) {
	SpawnTrailFX(hitResult);
	SpawnImpactFX(hitResult);
}

void AInstantWeapon::SpawnTrailFX(const FHitResult hitResult) {
	if(!TrailFX)
		return;
	auto endPoint = hitResult.bBlockingHit ? hitResult.ImpactPoint : hitResult.TraceEnd;
	SpawnTrailFX(endPoint, true);
	SpawnTrailFX(endPoint, false);
}


UParticleSystemComponent* AInstantWeapon::SpawnTrailFX(const FVector& end, bool isFirstPerson) {
	UParticleSystemComponent* fpTrail = UGameplayStatics::SpawnEmitterAttached(TrailFX, isFirstPerson ? Mesh1P : Mesh3P, NAME_None);
	if(!fpTrail)
		return nullptr;
	FVector start = isFirstPerson ? GetFPMuzzleLocation() : GetTPMuzzleLocation();
	fpTrail->bOwnerNoSee = !isFirstPerson;
	fpTrail->bOnlyOwnerSee = isFirstPerson;
	fpTrail->SetBeamSourcePoint(0, start, 0);
	fpTrail->SetBeamTargetPoint(0, end, 0);
	fpTrail->SetVectorParameter(TrailSourceParamName, start);
	fpTrail->SetVectorParameter(TrailTargetParamName, end);
	return fpTrail;
}

void AInstantWeapon::SpawnImpactFX(const FHitResult hitResult) {
	if(!hitResult.bBlockingHit)
		return;
	UParticleSystemComponent* tpTrail = UGameplayStatics::SpawnEmitterAtLocation(this, ImpactFX, hitResult.ImpactPoint);
	if(tpTrail) {
		tpTrail->SetWorldRotation(hitResult.ImpactNormal.ToOrientationRotator());
		tpTrail->SetRelativeScale3D(ImpactFXScale);
	}
}

void AInstantWeapon::OnEquip() {
	Super::OnEquip();
	// always start with a new RandomStream for replay Precision
	WeaponRandomStream = FRandomStream(0);
}

float AInstantWeapon::GetAIWeaponSuitability(ABaseCharacter* shooter, AActor* victim) {
	float suitability = Super::GetAIWeaponSuitability(shooter, victim);
	if(shooter && victim && (shooter->GetActorLocation() - victim->GetActorLocation()).Size() > Range)
		return 0;
	return suitability;
}