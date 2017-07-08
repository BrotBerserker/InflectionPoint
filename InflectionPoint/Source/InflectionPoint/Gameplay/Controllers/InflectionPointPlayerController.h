// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/PlayerController.h"
#include "InflectionPointPlayerController.generated.h"

/**
 *
 */
UCLASS()
class INFLECTIONPOINT_API AInflectionPointPlayerController : public APlayerController {
	GENERATED_BODY()
public:
	AInflectionPointPlayerController(const FObjectInitializer& ObjectInitializer);


public:
	UPROPERTY(BlueprintReadWrite, Category = "InflectionPoint|Gameplay")
		int Team;

public:
	char PlayerStartGroup;
};
