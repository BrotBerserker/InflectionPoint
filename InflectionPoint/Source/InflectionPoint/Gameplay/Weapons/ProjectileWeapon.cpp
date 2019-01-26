// Fill out your copyright notice in the Description page of Project Settings.

#include "InflectionPoint.h"
#include "Gameplay/Characters/BaseCharacter.h"
#include "ProjectileWeapon.h"

AProjectileWeapon::AProjectileWeapon() {DebugPrint(__FILE__, __LINE__);
	AISuitabilityWeaponRangeCurve.GetRichCurve()->AddKey(1000, 1.0);
	TargetBeam = CreateDefaultSubobject<UParticleSystemComponent>(TEXT("TargetBeam"));
	TargetBeam->SetupAttachment(Mesh1P);
DebugPrint(__FILE__, __LINE__);}

void AProjectileWeapon::Tick(float DeltaTime) {DebugPrint(__FILE__, __LINE__);
	Super::Tick(DeltaTime);

	if(ProjectileClass.GetDefaultObject()->Homing && CurrentAmmoInClip > 0) {DebugPrint(__FILE__, __LINE__);
		UPrimitiveComponent* newTarget = FindSelectedTarget();
		if(newTarget != SelectedTargetComponent) {DebugPrint(__FILE__, __LINE__);
			SwitchSelectedTarget(newTarget);
		DebugPrint(__FILE__, __LINE__);}

		if(TargetShouldBeDeselected(SelectedTargetComponent)) {DebugPrint(__FILE__, __LINE__);
			SwitchSelectedTarget(NULL);
		DebugPrint(__FILE__, __LINE__);}

		UpdateTargetBeam();
	DebugPrint(__FILE__, __LINE__);} else {DebugPrint(__FILE__, __LINE__);
		SwitchSelectedTarget(NULL);
		TargetBeam->Deactivate();
	DebugPrint(__FILE__, __LINE__);}
DebugPrint(__FILE__, __LINE__);}

bool AProjectileWeapon::TargetShouldBeDeselected(UPrimitiveComponent* targetComponent) {DebugPrint(__FILE__, __LINE__);
	if(!targetComponent || !Cast<ABaseCharacter>(targetComponent->GetOwner())) {DebugPrint(__FILE__, __LINE__);
		return false;
	DebugPrint(__FILE__, __LINE__);}
	if(!Cast<ABaseCharacter>(targetComponent->GetOwner())->IsAlive()) {DebugPrint(__FILE__, __LINE__);
		return true;
	DebugPrint(__FILE__, __LINE__);} else if(GetOwner() && GetOwner()->GetDistanceTo(targetComponent->GetOwner()) > TargetSelectingRange) {DebugPrint(__FILE__, __LINE__);
		return true;
	DebugPrint(__FILE__, __LINE__);} else if(Cast<ABaseCharacter>(GetOwner())->Controller && !Cast<ABaseCharacter>(GetOwner())->Controller->LineOfSightTo(targetComponent->GetOwner())) {DebugPrint(__FILE__, __LINE__);
		return true;
	DebugPrint(__FILE__, __LINE__);}
	return false;
DebugPrint(__FILE__, __LINE__);}

void AProjectileWeapon::SwitchSelectedTarget(UPrimitiveComponent * newTarget) {DebugPrint(__FILE__, __LINE__);
	if(GetOwner() && Cast<ABaseCharacter>(GetOwner())->IsLocallyControlled() && !GetOwner()->FindComponentByClass<UCharacterInfoProvider>()->GetCharacterInfo().IsReplay) {DebugPrint(__FILE__, __LINE__);
		UnMarkTarget(SelectedTargetComponent);
		MarkTarget(newTarget);
	DebugPrint(__FILE__, __LINE__);}
	SelectedTargetComponent = newTarget;
DebugPrint(__FILE__, __LINE__);}

UPrimitiveComponent* AProjectileWeapon::FindSelectedTarget() {DebugPrint(__FILE__, __LINE__);
	if(!Cast<ABaseCharacter>(GetOwner()) || !Cast<ABaseCharacter>(GetOwner())->FirstPersonCameraComponent) {DebugPrint(__FILE__, __LINE__);
		return NULL;
	DebugPrint(__FILE__, __LINE__);}
	FVector StartLocation = Cast<ABaseCharacter>(GetOwner())->FirstPersonCameraComponent->GetComponentLocation();
	FVector EndLocation = StartLocation + Cast<ABaseCharacter>(GetOwner())->FirstPersonCameraComponent->GetForwardVector() * TargetSelectingRange;
	FCollisionShape Shape = FCollisionShape::MakeBox(FVector(30, 30, 50));
	FQuat ShapeRotation = FQuat(0, 0, 0, 0);
	FCollisionObjectQueryParams ObjectQueryParams;
	ObjectQueryParams.AddObjectTypesToQuery(ECollisionChannel::ECC_Pawn);
	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(GetOwner());
	FHitResult SweepResult;
	bool hit = GetWorld()->SweepSingleByObjectType(SweepResult, StartLocation, EndLocation, ShapeRotation, ObjectQueryParams, Shape, QueryParams);
	if(hit && SweepResult.Component.IsValid()) {DebugPrint(__FILE__, __LINE__);
		if(!Cast<ABaseCharacter>(SweepResult.Actor.Get())->IsAlive()) {DebugPrint(__FILE__, __LINE__);
			return NULL;
		DebugPrint(__FILE__, __LINE__);}
		return SweepResult.Component.Get();
	DebugPrint(__FILE__, __LINE__);}
	return NULL;
DebugPrint(__FILE__, __LINE__);}

void AProjectileWeapon::MarkTarget(UPrimitiveComponent* targetComponent) {DebugPrint(__FILE__, __LINE__);
	if(!targetComponent) {DebugPrint(__FILE__, __LINE__);
		return;
	DebugPrint(__FILE__, __LINE__);}
	if(Cast<ABaseCharacter>(targetComponent->GetOwner())) {DebugPrint(__FILE__, __LINE__);
		Cast<ABaseCharacter>(targetComponent->GetOwner())->TargetMarkerParticles->SetVisibility(true);
	DebugPrint(__FILE__, __LINE__);}
DebugPrint(__FILE__, __LINE__);}

void AProjectileWeapon::UnMarkTarget(UPrimitiveComponent* targetComponent) {DebugPrint(__FILE__, __LINE__);
	if(!targetComponent) {DebugPrint(__FILE__, __LINE__);
		return;
	DebugPrint(__FILE__, __LINE__);}
	if(Cast<ABaseCharacter>(targetComponent->GetOwner())) {DebugPrint(__FILE__, __LINE__);
		Cast<ABaseCharacter>(targetComponent->GetOwner())->TargetMarkerParticles->SetVisibility(false);
	DebugPrint(__FILE__, __LINE__);}
DebugPrint(__FILE__, __LINE__);}

void AProjectileWeapon::UpdateTargetBeam() {DebugPrint(__FILE__, __LINE__);
	if(SelectedTargetComponent) {DebugPrint(__FILE__, __LINE__);
		TargetBeam->SetBeamTargetPoint(0, SelectedTargetComponent->GetComponentLocation(), 0);
		TargetBeam->SetBeamSourcePoint(0, Mesh1P->GetComponentLocation(), 0);
		TargetBeam->Activate();
	DebugPrint(__FILE__, __LINE__);} else {DebugPrint(__FILE__, __LINE__);
		TargetBeam->Deactivate();
	DebugPrint(__FILE__, __LINE__);}
DebugPrint(__FILE__, __LINE__);}

void AProjectileWeapon::OnUnequip() {DebugPrint(__FILE__, __LINE__);
	Super::OnUnequip();
	SwitchSelectedTarget(NULL);
DebugPrint(__FILE__, __LINE__);}

bool AProjectileWeapon::CanFire() {DebugPrint(__FILE__, __LINE__);
	return !ProjectileClass.GetDefaultObject()->Homing || SelectedTargetComponent != NULL;
DebugPrint(__FILE__, __LINE__);}

void AProjectileWeapon::ExecuteFire() {DebugPrint(__FILE__, __LINE__);
	UWorld* const World = GetWorld();
	if(ProjectileClass == NULL || !AssertNotNull(World, GetWorld(), __FILE__, __LINE__))
		return;

	//Set Spawn Collision Handling Override
	FActorSpawnParameters ActorSpawnParams;
	ActorSpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	ActorSpawnParams.Instigator = Instigator;
	ActorSpawnParams.Owner = this;

	// spawn the projectile at the muzzle
	AInflectionPointProjectile* projectile = World->SpawnActor<AInflectionPointProjectile>(ProjectileClass, GetFPMuzzleLocation(), GetAimDirection(), ActorSpawnParams);
	projectile->SetHomingTarget(SelectedTargetComponent);
DebugPrint(__FILE__, __LINE__);}