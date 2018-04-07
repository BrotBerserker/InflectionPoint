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
	// Cheats: https://docs.unrealengine.com/latest/INT/BlueprintAPI/CheatManager/
	// Use the "flushpersistentdebuglines" command to clear debug lines
public:

	UFUNCTION(exec)
		virtual void EnableDebugProjectileLineTrace(bool enable = true);

	UFUNCTION(exec)
		virtual void EnableCharacterDebugArrows(bool enable = true);

	UFUNCTION(exec)
		virtual void EnableLineTracing(bool enable = true);

	UFUNCTION(exec)
		virtual void EnableAllReplayDebugging(bool enable = true);

	UFUNCTION(exec)
		virtual void EnableHUD(bool enable = true);

	UPROPERTY(BlueprintReadWrite)
		bool IsDebugProjectileLineTraceEnabled = false;

	UPROPERTY(BlueprintReadWrite)
		bool IsCharacterDebugArrowsEnabled = false;
	
	UPROPERTY(BlueprintReadWrite)
		bool IsLineTracingEnabled = false;

	UPROPERTY(BlueprintReadWrite)
		bool IsHUDEnabled = true;
};
