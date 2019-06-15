// Fill out your copyright notice in the Description page of Project Settings.

#include "InflectionPoint.h"
#include "Gameplay/Weapons/BaseWeapon.h"
#include "ProjectileWeaponModule.h"

UProjectileWeaponModule::UProjectileWeaponModule() {
}

void UProjectileWeaponModule::Tick(float DeltaTime) {
	Super::Tick(DeltaTime);
	if(ProjectileClass != NULL && ProjectileClass.GetDefaultObject()->Homing && Weapon->CurrentAmmoInClip > 0) { 
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
		TargetBeamComponent->Deactivate();
	}
}

bool UProjectileWeaponModule::TargetShouldBeDeselected(UPrimitiveComponent* targetComponent) {
	if(!targetComponent || !Cast<ABaseCharacter>(targetComponent->GetOwner())) {
		return false;
	}
	if(!Cast<ABaseCharacter>(targetComponent->GetOwner())->IsAlive()) {
		return true;
	} else if(OwningCharacter && OwningCharacter->GetDistanceTo(targetComponent->GetOwner()) > TargetSelectingRange) {
		return true;
	} else if(OwningCharacter && OwningCharacter->Controller && !OwningCharacter->Controller->LineOfSightTo(targetComponent->GetOwner())) {
		return true;
	}
	return false;
}

void UProjectileWeaponModule::SwitchSelectedTarget(UPrimitiveComponent * newTarget) {
	if(OwningCharacter && OwningCharacter->IsLocallyControlled() && !OwningCharacter->FindComponentByClass<UCharacterInfoProvider>()->GetCharacterInfo().IsReplay) {
		UnMarkTarget(SelectedTargetComponent);
		MarkTarget(newTarget);
	}
	SelectedTargetComponent = newTarget;
}

UPrimitiveComponent* UProjectileWeaponModule::FindSelectedTarget() {
	if(!OwningCharacter || !OwningCharacter->FirstPersonCameraComponent) {
		return NULL;
	}
	FVector StartLocation = OwningCharacter->FirstPersonCameraComponent->GetComponentLocation();
	FVector EndLocation = StartLocation + OwningCharacter->FirstPersonCameraComponent->GetForwardVector() * TargetSelectingRange;
	FCollisionShape Shape = FCollisionShape::MakeBox(FVector(30, 30, 50));
	FQuat ShapeRotation = FQuat(0, 0, 0, 0);
	FCollisionObjectQueryParams ObjectQueryParams;
	ObjectQueryParams.AddObjectTypesToQuery(ECollisionChannel::ECC_Pawn);
	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(OwningCharacter);
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

void UProjectileWeaponModule::MarkTarget(UPrimitiveComponent* targetComponent) {
	if(!targetComponent) {
		return;
	}
	if(Cast<ABaseCharacter>(targetComponent->GetOwner())) {
		Cast<ABaseCharacter>(targetComponent->GetOwner())->TargetMarkerParticles->SetVisibility(true);
	}
}

void UProjectileWeaponModule::UnMarkTarget(UPrimitiveComponent* targetComponent) {
	if(!targetComponent) {
		return;
	}
	if(Cast<ABaseCharacter>(targetComponent->GetOwner())) {
		Cast<ABaseCharacter>(targetComponent->GetOwner())->TargetMarkerParticles->SetVisibility(false);
	}
}

void UProjectileWeaponModule::UpdateTargetBeam() {
	if(SelectedTargetComponent) {
		TargetBeamComponent->SetBeamTargetPoint(0, SelectedTargetComponent->GetComponentLocation(), 0);
		TargetBeamComponent->SetBeamSourcePoint(0, Weapon->Mesh1P->GetComponentLocation(), 0);
		TargetBeamComponent->Activate();
	} else {
		TargetBeamComponent->Deactivate();
	}
}

void UProjectileWeaponModule::Initialize() {
	Super::Initialize();
	TargetBeamComponent = UGameplayStatics::SpawnEmitterAttached(TargetBeam, Weapon->Mesh1P, NAME_None);
}

void UProjectileWeaponModule::OnActivate() {
	Super::OnActivate();
}

void UProjectileWeaponModule::OnDeactivate() {
	Super::OnDeactivate();
	SwitchSelectedTarget(NULL);
}

bool UProjectileWeaponModule::CanFire() {
	if(ProjectileClass == NULL)
		return false;
	return !ProjectileClass.GetDefaultObject()->Homing || SelectedTargetComponent != NULL;
}

void UProjectileWeaponModule::ExecuteFire() {
	UWorld* const World = GetWorld();
	if(ProjectileClass == NULL || !AssertNotNull(World, GetWorld(), __FILE__, __LINE__))
		return;

	//Set Spawn Collision Handling Override
	FActorSpawnParameters ActorSpawnParams;
	ActorSpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	ActorSpawnParams.Instigator = Weapon->Instigator;
	ActorSpawnParams.Owner = Weapon;

	// spawn the projectile at the muzzle
	AInflectionPointProjectile* projectile = World->SpawnActor<AInflectionPointProjectile>(ProjectileClass, Weapon->GetFPMuzzleLocation(), Weapon->GetAimDirection(), ActorSpawnParams);
	projectile->SetHomingTarget(SelectedTargetComponent);
}