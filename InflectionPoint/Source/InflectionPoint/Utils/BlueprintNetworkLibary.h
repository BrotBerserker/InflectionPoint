// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Kismet/BlueprintFunctionLibrary.h"
#include "BlueprintNetworkLibary.generated.h"

/**
 *
 */
UCLASS()
class UBlueprintNetworkLibary : public UBlueprintFunctionLibrary {
	GENERATED_UCLASS_BODY()

	UFUNCTION(BlueprintCallable, Category = "InflectionPoint|Networking")
		static void LeaveMultiplayerGame(FName SessionName);

};