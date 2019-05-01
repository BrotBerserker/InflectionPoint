// Fill out your copyright notice in the Description page of Project Settings.

#include "InflectionPoint.h"
#include "BuildableActor.h"

ABuildableActor::ABuildableActor() {
	PrimaryActorTick.bCanEverTick = false;

	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	Mesh->bCastDynamicShadow = false;
	Mesh->CastShadow = false;
	Mesh->SetCollisionProfileName("OverlapOnlyPawn");

	RootComponent = Mesh;

	MaterializeTimeline = CreateDefaultSubobject<UTimelineComponent>(TEXT("MaterializeTimeline"));
}

void ABuildableActor::BeginPlay() {
	Super::BeginPlay();
	PreviewMaterialInstance = Mesh->CreateAndSetMaterialInstanceDynamicFromMaterial(0, PreviewMaterial);
}

void ABuildableActor::UpdateLocation(FVector NewLocation, FVector HitNormal, AActor* HitActor) {
	if(!GetOwner()) {
		return;
	}
	SetActorLocation(GetOwner()->GetActorLocation());
	FHitResult hitResult;
	SetActorLocation(NewLocation, true, &hitResult);
	SetActorLocation(NewLocation);
	FRotator rotator = FRotationMatrix::MakeFromZY(HitNormal, GetOwner()->GetRootComponent()->GetForwardVector()* (-1)).Rotator();
	SetActorRotation(rotator);
	if(hitResult.Actor != nullptr && HitActor != nullptr && hitResult.Actor->GetName() != HitActor->GetName()) {
		DisableBuild();
	} else {
		EnableBuild();
	}
}

bool ABuildableActor::ServerBuild_Validate() {
	return true;
}

void ABuildableActor::ServerBuild_Implementation() {
	SetReplicates(true);
	SetReplicateMovement(true);
	Mesh->SetIsReplicated(true);
	MulticastBuild();
}

void ABuildableActor::MulticastBuild_Implementation() {
	Mesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Block);
	MaterializeMaterialInstance = Mesh->CreateAndSetMaterialInstanceDynamicFromMaterial(0, MaterializeMaterial);

	FOnTimelineFloat callback{};
	callback.BindUFunction(this, FName{ TEXT("MaterializeCallback") });
	MaterializeTimeline->AddInterpFloat(MaterializeCurve, callback, FName{ TEXT("MaterializeTimelineAnimation") });

	MaterializeTimeline->Play();
}

void ABuildableActor::MaterializeCallback(float Value) {
	MaterializeMaterialInstance->SetScalarParameterValue("Materialize Amount", Value);
}

void ABuildableActor::MaterializeFinishCallback() {
	Mesh->SetMaterial(0, FinishedMaterial);
}

void ABuildableActor::EnableBuild() {
	PreviewMaterialInstance->SetVectorParameterValue("BuildingPossibleColor", FVector(1.f, 0.7f, 0.f));
	CanBeBuilt = true;
}

void ABuildableActor::DisableBuild() {
	PreviewMaterialInstance->SetVectorParameterValue("BuildingPossibleColor", FVector(1.f, 0.f, 0.f));
	CanBeBuilt = false;
}

