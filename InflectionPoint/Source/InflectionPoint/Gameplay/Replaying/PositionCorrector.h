// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Components/ActorComponent.h"
#include "PositionCorrector.generated.h"

// ----------------------
// | (!) NOT USED!  (!) |
// ----------------------

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class INFLECTIONPOINT_API UPositionCorrector : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UPositionCorrector();

	// Called when the game starts
	virtual void BeginPlay() override;

	void StartCorrecting(TArray<TPair<float, FVector>> positions);

	UFUNCTION()
		void CorrectPosition(FVector correctPosition);

	UPROPERTY(EditAnywhere, Category = General)
		float CorrectionRadius = 1.f;

	UPROPERTY(EditAnywhere, Category = Debug)
		bool CreateDebugCorrectionSpheres = false;

	UPROPERTY(EditAnywhere, Category = Debug)
		FColor DebugHitColor = FColorList::Yellow;

	UPROPERTY(EditAnywhere, Category = Debug)
		FColor DebugMissColor = FColorList::LightSteelBlue;
};
