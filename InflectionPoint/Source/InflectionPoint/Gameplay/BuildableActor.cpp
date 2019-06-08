// Fill out your copyright notice in the Description page of Project Settings.

#include "InflectionPoint.h"
#include "Gameplay/Damage/MortalityProvider.h"
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
		NewComponent->SetCollisionProfileName("BuildingPreview");

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
	UMortalityProvider* mortalityProvider = GetOwner()->FindComponentByClass<UMortalityProvider>();
	if(mortalityProvider) {
		mortalityProvider->OnDeath.AddDynamic(this, &ABuildableActor::OnOwnerDeath);
	}
}

void ABuildableActor::OnOwnerDeath(AController* KillingPlayer, AActor* DamageCauser) {
	if(CurrentStage == -1) {
		Destroy();
	}
	if(TargetBuilding) {
		TargetBuilding->HideNextStagePreview();
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
		ShowPreviewAtLocation(NewLocation, HitNormal);
		DisableBuild();
	}
}

void ABuildableActor::UpdateLocationOnMap(FVector &NewLocation, FVector &HitNormal, AActor * HitActor) {
	ClearTargetBuilding();
	FHitResult hitResult = SweepToLocation(NewLocation);
	ShowPreviewAtLocation(NewLocation, HitNormal);
	if(CollidesWithExistingBuilding(hitResult)) {
		UpdateLocationOnExistingBuilding(hitResult.Actor.Get(), NewLocation, HitNormal);
	} else if(CollidesWithOtherActor(hitResult, HitActor)) {
		DisableBuild();
	} else {
		EnableBuild();
	}
}

bool ABuildableActor::CollidesWithExistingBuilding(FHitResult &hitResult) {
	return hitResult.Actor.IsValid() && hitResult.Actor->IsA(ABuildableActor::StaticClass());
}

bool ABuildableActor::CollidesWithOtherActor(FHitResult &hitResult, AActor * HitActor) {
	return hitResult.Actor.IsValid() && HitActor != nullptr && hitResult.Actor->GetName() != HitActor->GetName();
}

void ABuildableActor::ShowPreviewAtLocation(FVector & NewLocation, FVector & HitNormal) {
	SetActorLocation(NewLocation);
	SetActorRotation(GetRotationFromHitNormal(HitNormal));
	ShowNextStagePreview();
}

void ABuildableActor::ClearTargetBuilding() {
	if(TargetBuilding) {
		TargetBuilding->HideNextStagePreview();
		TargetBuilding = nullptr;
	}
}

FHitResult ABuildableActor::SweepToLocation(FVector & NewLocation) {
	SetActorLocation(GetOwner()->GetActorLocation());
	FHitResult hitResult;
	SetActorLocation(NewLocation, true, &hitResult);
	return hitResult;
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

	UGameplayStatics::SpawnSoundAttached(BuildSound, RootComponent);

	StageMeshes[CurrentStage]->SetVisibility(true);
	StageMeshes[CurrentStage]->SetCollisionProfileName("Building");
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
	ClearTargetBuilding();
}
