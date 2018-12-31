// Fill out your copyright notice in the Description page of Project Settings.

#include "InflectionPoint.h"
#include "Gameplay/Characters/BaseCharacter.h"
#include "ProjectileWeapon.h"

AProjectileWeapon::AProjectileWeapon() {
	AISuitabilityWeaponRangeCurve.GetRichCurve()->AddKey(1000, 1.0);
}

void AProjectileWeapon::Tick(float DeltaTime) {
	Super::Tick(DeltaTime);
	if(ProjectileClass.GetDefaultObject()->Homing) {
		UpdateSelectedTarget();
	} else if(SelectedTargetComponent) {
		SetTargetMarkerVisibility(SelectedTargetComponent->GetOwner(), false);
	}
}

void AProjectileWeapon::UpdateSelectedTarget() {
	FVector StartLocation = GetWorld()->GetFirstPlayerController()->PlayerCameraManager->GetCameraLocation();
	FVector EndLocation = StartLocation + GetWorld()->GetFirstPlayerController()->PlayerCameraManager->GetActorForwardVector() * 3000;
	FCollisionShape Shape = FCollisionShape::MakeBox(FVector(25, 1, 1));
	FQuat ShapeRotation = FQuat(0, 0, 0, 0);
	FCollisionObjectQueryParams ObjectQueryParams;
	ObjectQueryParams.AddObjectTypesToQuery(ECollisionChannel::ECC_Pawn);
	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(GetOwner());
	FHitResult SweepResult;
	bool hit = GetWorld()->SweepSingleByObjectType(SweepResult, StartLocation, EndLocation, ShapeRotation, ObjectQueryParams, Shape, QueryParams);
	if(hit) {
		if(SelectedTargetComponent) {
			SetTargetMarkerVisibility(SelectedTargetComponent->GetOwner(), false);
		}
		SetTargetMarkerVisibility(SweepResult.Actor.Get(), true);
		SelectedTargetComponent = SweepResult.Component.Get();
	}
}

void AProjectileWeapon::SetTargetMarkerVisibility(AActor* actor, bool visible) {
	if(Cast<ABaseCharacter>(actor)) {
		Cast<ABaseCharacter>(actor)->TargetMarker->SetVisibility(visible);
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