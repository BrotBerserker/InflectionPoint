// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/LevelScriptActor.h"
#include "TDMLevelScriptBase.generated.h"

/**
 *
 */
UCLASS()
class INFLECTIONPOINT_API ATDMLevelScriptBase : public ALevelScriptActor {
	GENERATED_BODY()
public:
	UFUNCTION(Reliable, NetMulticast)
		void MulticastStartSpawnCinematic();
private:
	int GetTeam();

};
