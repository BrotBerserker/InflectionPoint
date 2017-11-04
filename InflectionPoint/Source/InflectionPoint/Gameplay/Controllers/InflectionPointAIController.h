// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "Gameplay/NameProvider.h"
#include "InflectionPointAIController.generated.h"

/**
 *
 */
UCLASS()
class INFLECTIONPOINT_API AInflectionPointAIController : public AAIController {
	GENERATED_BODY()

public:
	AInflectionPointAIController(const FObjectInitializer& ObjectInitializer);

	void Initialize();

	UPROPERTY()
		class UNameProvider* NameProvider;

};
