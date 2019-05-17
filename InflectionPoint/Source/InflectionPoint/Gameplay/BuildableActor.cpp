// Fill out your copyright notice in the Description page of Project Settings.

#include "InflectionPoint.h"
#include "BuildableActor.h"

ABuildableActor::ABuildableActor() {
	PrimaryActorTick.bCanEverTick = false;
	MaterializeTimeline = CreateDefaultSubobject<UTimelineComponent>(TEXT("MaterializeTimeline"));
}

void ABuildableActor::OnConstruction(const FTransform & Transform) {
	Super::OnConstruction(Transform);
	for(int i = 0; i < BuildingStages.Num(); i++) {
		UStaticMeshComponent* NewComponent = NewObject<UStaticMeshComponent>(this, UStaticMeshComponent::StaticClass(), *FString("Stage " + FString::FromInt(i)));

		NewComponent->RegisterComponent();
		NewComponent->OnComponentCreated(); // Might need this line, might not.

		NewComponent->AttachToComponent(GetRootComponent(), FAttachmentTransformRules::KeepWorldTransform);
		NewComponent->SetStaticMesh(BuildingStages[i].Mesh);
		NewComponent->SetRelativeTransform(BuildingStages[i].Transform);

		NewComponent->bCastDynamicShadow = false;
		NewComponent->CastShadow = false;
		NewComponent->SetCollisionProfileName("OverlapOnlyPawn");

		StageMeshes.Add(NewComponent);

		if(i == 0) {
			RootComponent = NewComponent;
		}
	}
}

void ABuildableActor::BeginPlay() {
	Super::BeginPlay();
	for(int i = 1; i < StageMeshes.Num(); i++) {
		StageMeshes[i]->SetVisibility(false);
	}
}

void ABuildableActor::UpdateLocation(FVector NewLocation, FVector HitNormal, AActor* HitActor) {
	if(!GetOwner()) {
		return;
	}
	if(HitActor->IsA(ABuildableActor::StaticClass())) {
		UpdateLocationOnExistingBuilding(HitActor, NewLocation, HitNormal);
	} else {
		UpdateLocationOnMap(NewLocation, HitNormal, HitActor);
	}
}

void ABuildableActor::UpdateLocationOnExistingBuilding(AActor * HitActor, FVector &NewLocation, FVector &HitNormal) {
	if(IsValidTargetBuilding(HitActor)) {
		TargetBuilding = Cast<ABuildableActor>(HitActor);
		TargetBuilding->ShowNextStagePreview();
		HideNextStagePreview();
		EnableBuild();
	} else {
		SetActorLocation(NewLocation);
		SetActorRotation(GetRotationFromHitNormal(HitNormal));
		ShowNextStagePreview();
		DisableBuild();
	}
}

void ABuildableActor::UpdateLocationOnMap(FVector &NewLocation, FVector &HitNormal, AActor * HitActor) {
	ShowNextStagePreview();
	if(TargetBuilding) {
		TargetBuilding->HideNextStagePreview();
		TargetBuilding = nullptr;
	}
	SetActorLocation(GetOwner()->GetActorLocation());
	FHitResult hitResult;
	SetActorLocation(NewLocation, true, &hitResult);
	SetActorLocation(NewLocation);
	SetActorRotation(GetRotationFromHitNormal(HitNormal));
	if(hitResult.Actor.IsValid() && hitResult.Actor->IsA(ABuildableActor::StaticClass())) {
		UpdateLocationOnExistingBuilding(hitResult.Actor.Get(), NewLocation, HitNormal);
	} else if(hitResult.Actor.IsValid() && HitActor != nullptr && hitResult.Actor->GetName() != HitActor->GetName()) {
		DisableBuild();
	} else {
		EnableBuild();
	}
}

const FRotator ABuildableActor::GetRotationFromHitNormal(FVector & HitNormal) {
	return FRotationMatrix::MakeFromZY(HitNormal, GetOwner()->GetRootComponent()->GetForwardVector()* (-1)).Rotator();
}

bool ABuildableActor::IsValidTargetBuilding(AActor * HitActor) {
	if(!HitActor->IsA(GetClass())) {
		return false;
	}
	ABuildableActor* buildable = Cast<ABuildableActor>(HitActor);
	if(buildable->CurrentStage == -1) {
		return false;
	}
	if(buildable->MaterializeTimeline->GetPlaybackPosition() < buildable->MaterializeTimeline->GetTimelineLength()) {
		return false;
	}
	return buildable->CurrentStage + 1 < buildable->BuildingStages.Num();
}

void ABuildableActor::ShowNextStagePreview() {
	if(StageMeshes.Num() > CurrentStage + 1) {
		PreviewMaterialInstance = StageMeshes[CurrentStage + 1]->CreateAndSetMaterialInstanceDynamicFromMaterial(0, PreviewMaterial);
		StageMeshes[CurrentStage + 1]->SetVisibility(true);
	}
}

void ABuildableActor::HideNextStagePreview() {
	if(StageMeshes.Num() > CurrentStage + 1) {
		StageMeshes[CurrentStage + 1]->SetVisibility(false);
	}
}

bool ABuildableActor::ServerBuild_Validate() {
	return true;
}

void ABuildableActor::ServerBuild_Implementation() {
	if(TargetBuilding) {
		TargetBuilding->ServerBuild();
		TargetBuilding = nullptr;
		Destroy();
	} else {
		SetReplicates(true);
		MulticastBuild();
	}
}

void ABuildableActor::MulticastBuild_Implementation() {
	CurrentStage++;

	StageMeshes[CurrentStage]->SetVisibility(true);
	StageMeshes[CurrentStage]->SetCollisionObjectType(ECollisionChannel::ECC_WorldStatic);
	StageMeshes[CurrentStage]->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Block);
	MaterializeMaterialInstance = StageMeshes[CurrentStage]->CreateAndSetMaterialInstanceDynamicFromMaterial(0, BuildingStages[CurrentStage].MaterializeMaterial);

	FOnTimelineFloat callback{};
	callback.BindUFunction(this, FName{ TEXT("MaterializeCallback") });
	MaterializeTimeline->AddInterpFloat(MaterializeCurve, callback, FName{ TEXT("MaterializeTimelineAnimation") });

	FOnTimelineEvent finishCallback{};
	finishCallback.BindUFunction(this, FName{ TEXT("MaterializeFinishCallback") });
	MaterializeTimeline->SetTimelineFinishedFunc(finishCallback);

	MaterializeTimeline->PlayFromStart();
}

void ABuildableActor::MaterializeCallback(float Value) {
	MaterializeMaterialInstance->SetScalarParameterValue("Materialize Amount", Value);
}

void ABuildableActor::MaterializeFinishCallback() {
	StageMeshes[CurrentStage]->SetMaterial(0, BuildingStages[CurrentStage].FinishedMaterial);
	if(CurrentStage + 1 == BuildingStages.Num()) {
		BuildingFinished();
	}
}

void ABuildableActor::EnableBuild() {
	CanBeBuilt = true;
	if(PreviewMaterialInstance) {
		PreviewMaterialInstance->SetVectorParameterValue("BuildingPossibleColor", FVector(1.f, 0.7f, 0.f));
	}
}

void ABuildableActor::DisableBuild() {
	CanBeBuilt = false;
	if(PreviewMaterialInstance) {
		PreviewMaterialInstance->SetVectorParameterValue("BuildingPossibleColor", FVector(1.f, 0.f, 0.f));
	}
}

void ABuildableActor::Destroyed() {
	Super::Destroyed();
	if(TargetBuilding) {
		TargetBuilding->HideNextStagePreview();
	}
}
