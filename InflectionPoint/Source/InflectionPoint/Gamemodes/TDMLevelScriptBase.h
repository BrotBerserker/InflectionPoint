// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/LevelScriptActor.h"
#include "Runtime/LevelSequence/Public/LevelSequenceActor.h"
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

	/**
	* Index is the Team number
	*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "InflectionPoint")
		TArray<ALevelSequenceActor*> SpawnCinematicLevelSequences;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "InflectionPoint")
		class ACameraActor* SpawnCinematicCamera;

	UFUNCTION(BlueprintImplementableEvent, Category = "InflectionPoint")
		void OnPrePlaySequence();	

	UFUNCTION()
		void ReturnCameraToPlayer();
	
	UFUNCTION(BlueprintImplementableEvent, Category = "InflectionPoint")
		void OnPostPlaySequence();

private:
	UFUNCTION(BlueprintCallable, Category = "InflectionPoint")
		int GetTeam();

};
