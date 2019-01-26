// Fill out your copyright notice in the Description page of Project Settings.

#include "InflectionPoint.h"
#include "DebugTools/InflectionPointCheatManager.h"
#include "Gameplay/Characters/BaseCharacter.h"
#include "Gameplay/Controllers/PlayerControllerBase.h"
#include "InstantWeapon.h"


AInstantWeapon::AInstantWeapon() {DebugPrint(__FILE__, __LINE__);
	DebugLineDrawer = CreateDefaultSubobject<UDebugLineDrawer>(TEXT("DebugLineDrawer"));

	// set default values
	AISuitabilityWeaponRangeCurve.GetRichCurve()->AddKey(Range >= 100 ? Range - 100 : 1, 1);
	AISuitabilityWeaponRangeCurve.GetRichCurve()->AddKey(Range >= 100 ? Range : 100, 0);
DebugPrint(__FILE__, __LINE__);}

void AInstantWeapon::PreExecuteFire() {DebugPrint(__FILE__, __LINE__);
	damageWasDealt = false;
DebugPrint(__FILE__, __LINE__);}

void AInstantWeapon::ExecuteFire() {DebugPrint(__FILE__, __LINE__);
	float spread = (OwningCharacter && OwningCharacter->IsAiming) ? AimSpread : Spread;
	const float ConeHalfAngle = FMath::DegreesToRadians(spread * 0.5f);

	const FVector direction = OwningCharacter->FirstPersonCameraComponent->GetForwardVector();
	const FVector StartTrace = OwningCharacter->FirstPersonCameraComponent->GetComponentLocation();
	const FVector ShootDir = WeaponRandomStream.VRandCone(direction, ConeHalfAngle, ConeHalfAngle);
	const FVector EndTrace = StartTrace + ShootDir * Range;

	FHitResult hitResult = WeaponTrace(StartTrace, EndTrace);
	MulticastSpawnInstantWeaponFX(hitResult);

	if(hitResult.bBlockingHit) {DebugPrint(__FILE__, __LINE__);
		DebugLineDrawer->DrawDebugLineTrace(GetFPMuzzleLocation(), hitResult.ImpactPoint);
	DebugPrint(__FILE__, __LINE__);}
	DealDamage(hitResult, ShootDir);
DebugPrint(__FILE__, __LINE__);}

void AInstantWeapon::PostExecuteFire() {DebugPrint(__FILE__, __LINE__);
	auto controller = Cast<APlayerControllerBase>(OwningCharacter->Controller);
	if(controller && damageWasDealt)
		controller->DamageDealt();
DebugPrint(__FILE__, __LINE__);}

FHitResult AInstantWeapon::WeaponTrace(const FVector& startTrace, const FVector& endTrace) {DebugPrint(__FILE__, __LINE__);
	static FName WeaponFireTag = FName(TEXT("WeaponTrace"));

	// Perform trace to retrieve hit info
	FCollisionQueryParams TraceParams(WeaponFireTag, true, Instigator);
	TraceParams.bTraceAsyncScene = true;
	TraceParams.bReturnPhysicalMaterial = true;

	FHitResult Hit(ForceInit);
	GetWorld()->LineTraceSingleByChannel(Hit, startTrace, endTrace, (ECollisionChannel)CollisionChannel, TraceParams);
	return Hit;
DebugPrint(__FILE__, __LINE__);}

void AInstantWeapon::DealDamage(const FHitResult hitResult, const FVector& ShootDir) {DebugPrint(__FILE__, __LINE__);
	if(!hitResult.Actor.IsValid())
		return;

	FPointDamageEvent PointDmg;
	PointDmg.DamageTypeClass = DamageType;
	PointDmg.HitInfo = hitResult;
	PointDmg.ShotDirection = ShootDir;
	PointDmg.Damage = Damage;

	// to notify a controller if a character was damaged
	if(hitResult.Actor.Get()->IsA(ABaseCharacter::StaticClass()))
		damageWasDealt = true;

	hitResult.GetActor()->TakeDamage(PointDmg.Damage, PointDmg, OwningCharacter->Controller, this);
DebugPrint(__FILE__, __LINE__);}

void AInstantWeapon::MulticastSpawnInstantWeaponFX_Implementation(const FHitResult hitResult) {DebugPrint(__FILE__, __LINE__);
	SpawnTrailFX(hitResult);
	SpawnImpactFX(hitResult);
DebugPrint(__FILE__, __LINE__);}

void AInstantWeapon::SpawnTrailFX(const FHitResult hitResult) {DebugPrint(__FILE__, __LINE__);
	if(!TrailFX)
		return;
	auto endPoint = hitResult.bBlockingHit ? hitResult.ImpactPoint : hitResult.TraceEnd;
	SpawnTrailFX(endPoint, true);
	SpawnTrailFX(endPoint, false);
DebugPrint(__FILE__, __LINE__);}


UParticleSystemComponent* AInstantWeapon::SpawnTrailFX(const FVector& end, bool isFirstPerson) {DebugPrint(__FILE__, __LINE__);
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
DebugPrint(__FILE__, __LINE__);}

void AInstantWeapon::SpawnImpactFX(const FHitResult hitResult) {DebugPrint(__FILE__, __LINE__);
	if(!hitResult.bBlockingHit)
		return;
	UParticleSystemComponent* tpTrail = UGameplayStatics::SpawnEmitterAtLocation(this, ImpactFX, hitResult.ImpactPoint);
	if(tpTrail)
		tpTrail->SetWorldRotation(hitResult.ImpactNormal.ToOrientationRotator());
DebugPrint(__FILE__, __LINE__);}

void AInstantWeapon::OnEquip() {DebugPrint(__FILE__, __LINE__);
	Super::OnEquip();
	// always start with a new RandomStream for replay Precision
	WeaponRandomStream = FRandomStream(0);
DebugPrint(__FILE__, __LINE__);}

float AInstantWeapon::GetAIWeaponSuitability(ABaseCharacter* shooter, AActor* victim) {DebugPrint(__FILE__, __LINE__);
	float suitability = Super::GetAIWeaponSuitability(shooter, victim);
	if(shooter && victim && (shooter->GetActorLocation() - victim->GetActorLocation()).Size() > Range)
		return 0;
	return suitability;
DebugPrint(__FILE__, __LINE__);}