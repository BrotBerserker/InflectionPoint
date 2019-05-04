// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "BuildableActor.generated.h"


USTRUCT(BlueprintType)
struct FBuildingStage {
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, EditAnywhere)
		UStaticMesh* Mesh;

	UPROPERTY(BlueprintReadOnly, EditAnywhere)
		UMaterialInterface* MaterializeMaterial;

	UPROPERTY(BlueprintReadOnly, EditAnywhere)
		UMaterialInterface* FinishedMaterial;

	UPROPERTY(BlueprintReadOnly, EditAnywhere)
		FTransform Transform;
};

UCLASS()
class INFLECTIONPOINT_API ABuildableActor : public AActor {
	GENERATED_BODY()

public:
	UTimelineComponent* MaterializeTimeline;

public:
	UPROPERTY(BlueprintReadOnly, EditAnywhere)
		TArray<FBuildingStage> BuildingStages;

	UPROPERTY(BlueprintReadOnly, EditAnywhere)
		UMaterialInterface* PreviewMaterial;

	UPROPERTY(EditAnywhere)
		UCurveFloat* MaterializeCurve;

	UPROPERTY(BlueprintReadOnly)
		TArray<UStaticMeshComponent*> StageMeshes;

	UPROPERTY(BlueprintReadOnly)
		int32 CurrentStage = -1;

public:
	ABuildableActor();

	virtual void BeginPlay() override;

	virtual void OnConstruction(const FTransform & Transform) override;

	virtual void Destroyed() override;

public:
	UFUNCTION(BlueprintCallable)
		void UpdateLocation(FVector NewLocation, FVector HitNormal, AActor* HitActor);

	UFUNCTION(BlueprintCallable)
		void ShowNextStagePreview();

	UFUNCTION(BlueprintCallable)
		void HideNextStagePreview();

	UFUNCTION(Server, Reliable, WithValidation)
		void ServerBuild();

	UFUNCTION(NetMulticast, Reliable)
		void MulticastBuild();

public:
	UFUNCTION()
		void MaterializeCallback(float Value);

	UFUNCTION()
		void MaterializeFinishCallback();

public:
	bool CanBeBuilt = false;

private:
	void EnableBuild();
	void DisableBuild();

	void UpdateLocationOnExistingBuilding(AActor * HitActor, FVector &NewLocation, FVector &HitNormal);
	const FRotator GetRotationFromHitNormal(FVector & HitNormal);
	bool IsValidTargetBuilding(AActor * HitActor);
	void UpdateLocationOnMap(FVector &NewLocation, FVector &HitNormal, AActor * HitActor);


private:
	ABuildableActor* TargetBuilding;

	UMaterialInstanceDynamic* PreviewMaterialInstance;
	UMaterialInstanceDynamic* MaterializeMaterialInstance;

};
