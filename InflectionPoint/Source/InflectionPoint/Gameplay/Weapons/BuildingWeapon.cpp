// Fill out your copyright notice in the Description page of Project Settings.

#include "InflectionPoint.h"
#include "Gameplay/Characters/BaseCharacter.h"
#include "BuildingWeapon.h"


ABuildingWeapon::ABuildingWeapon() {
	OutOfRangeText = CreateDefaultSubobject<UTextRenderComponent>(TEXT("OutOfRangeText"));
	OutOfRangeText->SetText(FText::FromString("Out of range!"));
	OutOfRangeText->SetRelativeLocation(FVector(8.78f, -1.66f, -11.13f));
	OutOfRangeText->SetRelativeRotation(FRotator(10.85f, -106.02f, 59.62f));
	OutOfRangeText->SetupAttachment(Mesh1P);
}

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
		UpdateBuildableActor(HitResult);
	} else {
		ShowOutOfRange();
	}
}

void ABuildingWeapon::UpdateBuildableActor(FHitResult &HitResult) {
	if(!buildableActor) {
		FActorSpawnParameters spawnParams;
		spawnParams.Owner = GetOwner();
		buildableActor = GetWorld()->SpawnActor<ABuildableActor>(BuildableActorClass, spawnParams);
	}
	buildableActor->UpdateLocation(HitResult.Location, HitResult.ImpactNormal, HitResult.Actor.Get());
	OutOfRangeText->SetVisibility(false);
}

void ABuildingWeapon::ShowOutOfRange() {
	if(buildableActor) {
		buildableActor->Destroy();
		buildableActor = nullptr;
	}
	OutOfRangeText->SetVisibility(true);
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