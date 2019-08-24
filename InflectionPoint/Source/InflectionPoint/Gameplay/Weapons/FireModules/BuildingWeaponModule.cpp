// Fill out your copyright notice in the Description page of Project Settings.

#include "InflectionPoint.h"
#include "BuildingWeaponModule.h"

//ABuildingWeapon::ABuildingWeapon() {
//	OutOfRangeText = CreateDefaultSubobject<UTextRenderComponent>(TEXT("OutOfRangeText"));
//	OutOfRangeText->SetText(FText::FromString("Out of range!"));
//	OutOfRangeText->SetRelativeLocation(FVector(8.78f, -1.66f, -11.13f));
//	OutOfRangeText->SetRelativeRotation(FRotator(10.85f, -106.02f, 59.62f));
//	OutOfRangeText->SetupAttachment(Mesh1P);
//}

void UBuildingWeaponModule::TickComponent(float DeltaTime, enum ELevelTick tickType, FActorComponentTickFunction *thisTickFunction) {
	Super::TickComponent(DeltaTime, tickType, thisTickFunction);
	if(!OwningCharacter || !OwningCharacter->FirstPersonCameraComponent || Weapon->CurrentAmmoInClip <= 0) {
		return;
	}
	FVector StartLocation = OwningCharacter->FirstPersonCameraComponent->GetComponentLocation();
	FVector EndLocation = StartLocation + OwningCharacter->FirstPersonCameraComponent->GetForwardVector() * BuildingRange;
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

void UBuildingWeaponModule::UpdateBuildableActor(FHitResult &HitResult) {
	if(!buildableActor) {
		FActorSpawnParameters spawnParams;
		spawnParams.Owner = OwningCharacter;
		buildableActor = GetWorld()->SpawnActor<ABuildableActor>(BuildableActorClass, spawnParams);
	}
	buildableActor->UpdateLocation(HitResult.Location, HitResult.ImpactNormal, HitResult.Actor.Get());
	//OutOfRangeText->SetVisibility(false);
}

void UBuildingWeaponModule::ShowOutOfRange() {
	if(buildableActor) {
		buildableActor->Destroy();
		buildableActor = nullptr;
	}
	//OutOfRangeText->SetVisibility(true);
}

void UBuildingWeaponModule::OnDeactivate() {
	Super::OnDeactivate();
	if(buildableActor) {
		buildableActor->Destroy();
		buildableActor = nullptr;
	}
}

bool UBuildingWeaponModule::CanFire() {
	return Super::CanFire() && buildableActor && buildableActor->CanBeBuilt;
}

void UBuildingWeaponModule::ExecuteFire() {
	MulticastHidePreview();
	if(buildableActor) {
		buildableActor->ServerBuild();
		buildableActor = nullptr;
	}
}

void UBuildingWeaponModule::MulticastHidePreview_Implementation() {
	if(!Weapon->HasAuthority()) {
		buildableActor->Destroy();
		buildableActor = nullptr;
	}
}
