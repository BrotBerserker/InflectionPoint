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

	if(!Cast<ABaseCharacter>(GetOwner())->IsLocallyControlled()) {
		return;
	}

	if(ProjectileClass.GetDefaultObject()->Homing) {
		UpdateSelectedTarget();
	}

	if(SelectedTargetComponent) {
		if(!Cast<ABaseCharacter>(SelectedTargetComponent->GetOwner())->IsAlive()) {
			SetTargetMarkerVisibility(SelectedTargetComponent->GetOwner(), false);
			SelectedTargetComponent = NULL;
		} else if(GetOwner()->GetDistanceTo(SelectedTargetComponent->GetOwner()) > 3000.f) {
			SetTargetMarkerVisibility(SelectedTargetComponent->GetOwner(), false);
			SelectedTargetComponent = NULL;
		} else if(!Cast<ABaseCharacter>(GetOwner())->Controller->LineOfSightTo(SelectedTargetComponent->GetOwner())) {
			SetTargetMarkerVisibility(SelectedTargetComponent->GetOwner(), false);
			SelectedTargetComponent = NULL;
		}
		TargetBeam->SetBeamTargetPoint(0, SelectedTargetComponent->GetComponentLocation(), 0);
		TargetBeam->SetBeamSourcePoint(0, Mesh1P->GetComponentLocation(), 0);
		TargetBeam->Activate();
	} else {
		TargetBeam->Deactivate();
	}
}

void AProjectileWeapon::UpdateSelectedTarget() {
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
	if(hit) {
		if(SelectedTargetComponent == SweepResult.Component.Get() || !Cast<ABaseCharacter>(SweepResult.Actor.Get())->IsAlive()) {
			return;
		}
		if(SelectedTargetComponent) {
			SetTargetMarkerVisibility(SelectedTargetComponent->GetOwner(), false);
		}
		SetTargetMarkerVisibility(SweepResult.Actor.Get(), true);
		SelectedTargetComponent = SweepResult.Component.Get();
		ServerSetSelectedTarget(SweepResult.Component.Get());
	}
}

bool AProjectileWeapon::ServerSetSelectedTarget_Validate(UPrimitiveComponent* NewTarget) {
	return true;
}

void AProjectileWeapon::ServerSetSelectedTarget_Implementation(UPrimitiveComponent* NewTarget) {
	SelectedTargetComponent = NewTarget;
}

void AProjectileWeapon::SetTargetMarkerVisibility(AActor* actor, bool visible) {
	if(GetOwner()->FindComponentByClass<UCharacterInfoProvider>()->GetCharacterInfo().IsReplay) {
		return;
	}
	if(Cast<ABaseCharacter>(actor)) {
		Cast<ABaseCharacter>(actor)->TargetMarkerParticles->SetVisibility(visible);
	}
}

void AProjectileWeapon::OnUnequip() {
	Super::OnUnequip();
	if(SelectedTargetComponent) {
		SetTargetMarkerVisibility(SelectedTargetComponent->GetOwner(), false);
		SelectedTargetComponent = NULL;
	}
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