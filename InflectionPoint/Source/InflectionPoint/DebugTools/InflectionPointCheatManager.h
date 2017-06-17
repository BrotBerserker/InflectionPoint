// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/CheatManager.h"
#include "GameFramework/CheatManager.h"
#include "InflectionPointCheatManager.generated.h"

/**
 *
 */
UCLASS()
class INFLECTIONPOINT_API UInflectionPointCheatManager : public UCheatManager {
	GENERATED_BODY()
		
		virtual void InitCheatManager() override;

	UFUNCTION(exec)
		virtual void PrintTest();
};
