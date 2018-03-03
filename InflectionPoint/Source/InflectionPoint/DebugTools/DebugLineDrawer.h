// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "DebugLineDrawer.generated.h"


UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class INFLECTIONPOINT_API UDebugLineDrawer : public UActorComponent {
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	UDebugLineDrawer();

public:
	// Called when the game starts
	virtual void BeginPlay() override;

	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	/* Color for Player debug Trace */
	UPROPERTY(EditAnywhere, Category = Debug)
		FColor PlayerDebugColor = FColor(10, 12, 160);

	/* Color for Replay debug Trace */
	UPROPERTY(EditAnywhere, Category = Debug)
		FColor ReplayDebugColor = FColor(160, 14, 0);

	/* The debug line thickness */
	UPROPERTY(EditAnywhere, Category = Debug)
		float LineThickness = 0.5f;

	/* Draws a debug line. Uses the replay color if the owner's instigator is a ReplayCharacter */
	UFUNCTION(BlueprintCallable)
		void DrawDebugLineTrace(FVector StartPos, FVector EndPos);

	/* Draws a debug line using the player color */
	UFUNCTION(BlueprintCallable)
		void DrawPlayerDebugLineTrace(FVector StartPos, FVector EndPos);

	/* Draws a debug line using the replay color */
	UFUNCTION(BlueprintCallable)
		void DrawReplayDebugLineTrace(FVector StartPos, FVector EndPos);

private:
	void DrawDebugLineTrace(FVector StartPos, FVector EndPos, bool IsReplay);

};
