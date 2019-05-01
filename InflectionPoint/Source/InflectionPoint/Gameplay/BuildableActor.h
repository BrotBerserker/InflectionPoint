// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "BuildableActor.generated.h"

UCLASS()
class INFLECTIONPOINT_API ABuildableActor : public AActor {
	GENERATED_BODY()

public:
	UTimelineComponent* MaterializeTimeline;

public:
	UPROPERTY(BlueprintReadOnly, EditAnywhere)
		UStaticMeshComponent* Mesh;

	UPROPERTY(BlueprintReadOnly, EditAnywhere)
		UMaterialInterface* PreviewMaterial;

	UPROPERTY(BlueprintReadOnly, EditAnywhere)
		UMaterialInterface* MaterializeMaterial;

	UPROPERTY(BlueprintReadOnly, EditAnywhere)
		UMaterialInterface* FinishedMaterial;

	UPROPERTY(EditAnywhere, Category = Materialize)
		UCurveFloat* MaterializeCurve;

public:
	// Sets default values for this actor's properties
	ABuildableActor();

	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	void UpdateLocation(FVector NewLocation, FVector HitNormal, AActor* HitActor);

public:
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

private:
	UMaterialInstanceDynamic* PreviewMaterialInstance;
	UMaterialInstanceDynamic* MaterializeMaterialInstance;

};
