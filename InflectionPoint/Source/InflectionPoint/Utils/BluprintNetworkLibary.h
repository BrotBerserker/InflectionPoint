// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Kismet/BlueprintFunctionLibrary.h"
#include "BluprintNetworkLibary.generated.h"

/**
 * 
 */
UCLASS()
class INFLECTIONPOINT_API UBluprintNetworkLibary : public UBlueprint {
	GENERATED_BODY()
public:
		UBluprintNetworkLibary();

	UFUNCTION(BlueprintImplementableEvent, Category = "InflectionPoint|Networking")
		void OnSessionDestroyed();

	UFUNCTION(BlueprintCallable, Category = "InflectionPoint|Networking")
		void LeaveMultiplayerGame(FName SessionName);

private:

	/** Delegate for destroying a session */
	FOnDestroySessionCompleteDelegate OnDestroySessionCompleteDelegate;

	/** Handle to registered delegate for destroying a session */
	FDelegateHandle OnDestroySessionCompleteDelegateHandle;

	void OnDestroySessionComplete(FName SessionName, bool bWasSuccessful);
};