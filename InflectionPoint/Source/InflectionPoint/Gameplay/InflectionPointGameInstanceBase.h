// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "InflectionPointGameInstanceBase.generated.h"

/**
 *
 */
UCLASS()
class INFLECTIONPOINT_API UInflectionPointGameInstanceBase : public UGameInstance {
	GENERATED_BODY()

public:
	UInflectionPointGameInstanceBase();

public:
	UPROPERTY(BlueprintReadWrite)
		FName CurrentSessionName;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
		class UInflectionPointSettings* IPSettings;
};
