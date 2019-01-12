// Fill out your copyright notice in the Description page of Project Settings.

#include "InflectionPoint.h"
#include "Gameplay/Characters/BaseCharacter.h"
#include "ProjectileWeapon.h"

AProjectileWeapon::AProjectileWeapon() {
	AISuitabilityWeaponRangeCurve.GetRichCurve()->AddKey(1000, 1.0);
	TargetBeam = CreateDefaultSubobject<UParticleSystemComponent>(TEXT("TargetBeam"));
	TargetBeam->SetupAttachment(Mesh1P);
}

void AProjectileWeapon::Tick(float DeltaTime) {
	Super::Tick(DeltaTime);

	if(ProjectileClass.GetDefaultObject()->Homing && CurrentAmmoInClip > 0) {
		UPrimitiveComponent* newTarget = FindSelectedTarget();
		if(newTarget != SelectedTargetComponent) {
			SwitchSelectedTarget(newTarget);
		}

		if(TargetShouldBeDeselected(SelectedTargetComponent)) {
			SwitchSelectedTarget(NULL);
		}

		UpdateTargetBeam();
	} else {
		SwitchSelectedTarget(NULL);
		TargetBeam->Deactivate();
	}
}

bool AProjectileWeapon::TargetShouldBeDeselected(UPrimitiveComponent* targetComponent) {
	if(!targetComponent || !Cast<ABaseCharacter>(targetComponent->GetOwner())) {
		return false;
	}
	if(!Cast<ABaseCharacter>(targetComponent->GetOwner())->IsAlive()) {
		return true;
	} else if(GetOwner() && GetOwner()->GetDistanceTo(targetComponent->GetOwner()) > 3000.f) {
		return true;
	} else if(Cast<ABaseCharacter>(GetOwner())->Controller && !Cast<ABaseCharacter>(GetOwner())->Controller->LineOfSightTo(targetComponent->GetOwner())) {
		return true;
	}
	return false;
}

void AProjectileWeapon::SwitchSelectedTarget(UPrimitiveComponent * newTarget) {
	if(Cast<ABaseCharacter>(GetOwner())->IsLocallyControlled() && !GetOwner()->FindComponentByClass<UCharacterInfoProvider>()->GetCharacterInfo().IsReplay) {
		UnMarkTarget(SelectedTargetComponent);
		MarkTarget(newTarget);
	}
	SelectedTargetComponent = newTarget;
}

UPrimitiveComponent* AProjectileWeapon::FindSelectedTarget() {
	FVector StartLocation = Cast<ABaseCharacter>(GetOwner())->FirstPersonCameraComponent->GetComponentLocation();
	FVector EndLocation = StartLocation + Cast<ABaseCharacter>(GetOwner())->FirstPersonCameraComponent->GetForwardVector() * 3000;
	FCollisionShape Shape = FCollisionShape::MakeBox(FVector(30, 30, 50));
	FQuat ShapeRotation = FQuat(0, 0, 0, 0);
	FCollisionObjectQueryParams ObjectQueryParams;
	ObjectQueryParams.AddObjectTypesToQuery(ECollisionChannel::ECC_Pawn);
	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(GetOwner());
	FHitResult SweepResult;
	bool hit = GetWorld()->SweepSingleByObjectType(SweepResult, StartLocation, EndLocation, ShapeRotation, ObjectQueryParams, Shape, QueryParams);
	if(hit && SweepResult.Component.IsValid()) {
		if(!Cast<ABaseCharacter>(SweepResult.Actor.Get())->IsAlive()) {
			return NULL;
		}
		return SweepResult.Component.Get();
	}
	return NULL;
}

void AProjectileWeapon::MarkTarget(UPrimitiveComponent* targetComponent) {
	if(!targetComponent) {
		return;
	}
	if(Cast<ABaseCharacter>(targetComponent->GetOwner())) {
		Cast<ABaseCharacter>(targetComponent->GetOwner())->TargetMarkerParticles->SetVisibility(true);
	}
}

void AProjectileWeapon::UnMarkTarget(UPrimitiveComponent* targetComponent) {
	if(!targetComponent) {
		return;
	}
	if(Cast<ABaseCharacter>(targetComponent->GetOwner())) {
		Cast<ABaseCharacter>(targetComponent->GetOwner())->TargetMarkerParticles->SetVisibility(false);
	}
}

void AProjectileWeapon::UpdateTargetBeam() {
	if(SelectedTargetComponent) {
		TargetBeam->SetBeamTargetPoint(0, SelectedTargetComponent->GetComponentLocation(), 0);
		TargetBeam->SetBeamSourcePoint(0, Mesh1P->GetComponentLocation(), 0);
		TargetBeam->Activate();
	} else {
		TargetBeam->Deactivate();
	}
}

void AProjectileWeapon::OnUnequip() {
	Super::OnUnequip();
	SwitchSelectedTarget(NULL);
}

bool AProjectileWeapon::CanFire() {
	return !ProjectileClass.GetDefaultObject()->Homing || SelectedTargetComponent != NULL;
}

void AProjectileWeapon::ExecuteFire() {
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
}