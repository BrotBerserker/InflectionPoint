// Fill out your copyright notice in the Description page of Project Settings.

#include "InflectionPoint.h"
#include "Gameplay/Characters/BaseCharacter.h"
#include "BuildingWeapon.h"

void ABuildingWeapon::Tick(float DeltaTime) {
	Super::Tick(DeltaTime);
	if(!Cast<ABaseCharacter>(GetOwner()) || !Cast<ABaseCharacter>(GetOwner())->FirstPersonCameraComponent || CurrentAmmoInClip <= 0) {
		return;
	}
	FVector StartLocation = Cast<ABaseCharacter>(GetOwner())->FirstPersonCameraComponent->GetComponentLocation();
	FVector EndLocation = StartLocation + Cast<ABaseCharacter>(GetOwner())->FirstPersonCameraComponent->GetForwardVector() * BuildingRange;
	FHitResult HitResult;
	FCollisionObjectQueryParams params;
	params.AddObjectTypesToQuery(ECollisionChannel::ECC_WorldStatic);
	bool hit = GetWorld()->LineTraceSingleByObjectType(HitResult, StartLocation, EndLocation, params);
	if(hit && HitResult.Actor.IsValid()) {
		if(!buildableActor) {
			FActorSpawnParameters spawnParams;
			spawnParams.Owner = GetOwner();
			buildableActor = GetWorld()->SpawnActor<ABuildableActor>(BuildableActorClass, spawnParams);
		}
		buildableActor->UpdateLocation(HitResult.Location, HitResult.ImpactNormal, HitResult.Actor.Get());
	} else if(buildableActor) {
		buildableActor->Destroy();
		buildableActor = nullptr;
	}
}

void ABuildingWeapon::OnEquip() {
	Super::OnEquip();

}

void ABuildingWeapon::OnUnequip() {
	Super::OnUnequip();
	if(buildableActor) {
		buildableActor->Destroy();
		buildableActor = nullptr;
	}
}

bool ABuildingWeapon::CanFire() {
	return buildableActor && buildableActor->CanBeBuilt;
}

void ABuildingWeapon::ExecuteFire() {
	MulticastHidePreview();
	if(buildableActor) {
		buildableActor->ServerBuild();
		buildableActor = nullptr;
	}
}

void ABuildingWeapon::MulticastHidePreview_Implementation() {
	if(!HasAuthority()) {
		buildableActor->Destroy();
		buildableActor = nullptr;
	}
}