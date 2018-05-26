// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "Gameplay/CharacterInfoProvider.h"
#include "AIControllerBase.generated.h"

/**
 *
 */
UCLASS()
class INFLECTIONPOINT_API AAIControllerBase : public AAIController {
	GENERATED_BODY()

public:
	AAIControllerBase(const FObjectInitializer& ObjectInitializer);

	UFUNCTION(BlueprintCallable)
		void Initialize(APlayerController* OwningController);

	UPROPERTY(BlueprintReadOnly)
		APlayerController* OwningPlayerController;
};
