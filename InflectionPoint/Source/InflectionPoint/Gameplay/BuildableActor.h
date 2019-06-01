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

	/** Material used to show a preview of the building when looking for a place to build */
	UPROPERTY(BlueprintReadOnly, EditAnywhere)
		UMaterialInterface* PreviewMaterial;

	UPROPERTY(EditAnywhere)
		UCurveFloat* MaterializeCurve;

	UPROPERTY(EditAnywhere)
		class USoundBase* BuildSound;

	UPROPERTY(BlueprintReadOnly)
		TArray<UStaticMeshComponent*> StageMeshes;

	/** The current stage (-1 = preview, 0 = the first stage has been built, etc.) */
	UPROPERTY(BlueprintReadOnly)
		int32 CurrentStage = -1;

public:
	ABuildableActor();

	virtual void BeginPlay() override;

	virtual void OnConstruction(const FTransform & Transform) override;

	virtual void Destroyed() override;

public:
	/** Updates this building's location and checks if an existing building should be upgraded instead */
	UFUNCTION(BlueprintCallable)
		void UpdateLocation(FVector NewLocation, FVector HitNormal, AActor* HitActor);

	/** Shows a preview for the next stage that has not been built yet */
	UFUNCTION(BlueprintCallable)
		void ShowNextStagePreview();

	/** Hides the preview for the next stage */
	UFUNCTION(BlueprintCallable)
		void HideNextStagePreview();

	/** Builds the next stage of this building or, if an existing building should be upgraded, builds that building's next stage */
	UFUNCTION(Server, Reliable, WithValidation)
		void ServerBuild();

	/** Plays the materialize timeline, updates visibilities etc. */
	UFUNCTION(NetMulticast, Reliable)
		void MulticastBuild();

	/** Called when the last stage has been built and the spawn animation has finished */
	UFUNCTION(BlueprintImplementableEvent)
		void BuildingFinished();

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
	void ShowPreviewAtLocation(FVector & NewLocation, FVector & HitNormal);
	const FRotator GetRotationFromHitNormal(FVector & HitNormal);
	bool IsValidTargetBuilding(AActor * HitActor);
	void UpdateLocationOnMap(FVector &NewLocation, FVector &HitNormal, AActor * HitActor);
	bool CollidesWithOtherActor(FHitResult &hitResult, AActor * HitActor);
	bool CollidesWithExistingBuilding(FHitResult &hitResult);
	void ClearTargetBuilding();
	FHitResult SweepToLocation(FVector & NewLocation);

private:
	ABuildableActor* TargetBuilding;

	UMaterialInstanceDynamic* PreviewMaterialInstance;
	UMaterialInstanceDynamic* MaterializeMaterialInstance;

};
