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
	/* ------------------- */
	/*   Spawn cinematic   */
	/* ------------------- */
	UFUNCTION(Reliable, NetMulticast)
		void MulticastStartSpawnCinematic();

	/** Sequence to play before a phase starts, one for each team */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "InflectionPoint|Spawn Cinematic")
		TArray<ALevelSequenceActor*> SpawnCinematicLevelSequences;

	/** Camera to use for spawn cinematic */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "InflectionPoint|Spawn Cinematic")
		class ACameraActor* SpawnCinematicCamera;

	UFUNCTION(BlueprintImplementableEvent, Category = "InflectionPoint|Spawn Cinematic")
		void OnPrePlaySpawnCinematic();

public:
	/* --------------- */
	/*    Match end    */
	/* --------------- */
	UFUNCTION(BlueprintCallable) // TODO blueprintcallable weg
		void StartEndMatchSequence(TSubclassOf<AActor> WinningActor, TSubclassOf<AActor> LosingActor, FString WinnerName, FString LoserName);

	UFUNCTION(Reliable, NetMulticast)
		void MulticastStartEndMatchSequence(const FString& winnerName, const FString& loserName);

	/** Sequence to play after a match has ended */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "InflectionPoint|Match End")
		ALevelSequenceActor* MatchEndLevelSequence;

	/** Camera to use at match end */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "InflectionPoint|Match End")
		class ACameraActor* MatchEndCamera;

	/** Where to spawn the winning player */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "InflectionPoint|Match End")
		AActor* WinningPlayerLocation;

	/** Where to spawn the losing player */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "InflectionPoint|Match End")
		AActor* LosingPlayerLocation;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "InflectionPoint|Match End")
		UAnimationAsset* WinningPlayerAnimation;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "InflectionPoint|Match End")
		UAnimationAsset* LosingPlayerAnimation;

private:
	AActor* SpawnActorForEndMatchSequence(TSubclassOf<AActor> actorToSpawn, AActor* location);

	void PrepareActorForEndMatchSequence(AActor * spawnedActor, UAnimationAsset* animation);

	void UpdateNameTag(AActor* location, FString name);

	void PrepareAndStartSequence(ACameraActor* camera, ALevelSequenceActor* sequenceActor, float fadeTime);

	UFUNCTION()
		void StartSequence(APlayerController* controller, ACameraActor* camera, ALevelSequenceActor* sequenceActor, float fadeTime);

	UFUNCTION(BlueprintCallable, Category = "InflectionPoint")
		int GetTeam();

	UFUNCTION()
		void ReturnCameraToPlayer();
};
