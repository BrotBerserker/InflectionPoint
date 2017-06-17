// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Blueprint/UserWidget.h"
#include "IngameMenuBase.generated.h"

/**
 * 
 */
UCLASS()
class INFLECTIONPOINT_API UIngameMenuBase : public UUserWidget
{
	GENERATED_BODY()

public:
	UIngameMenuBase(const FObjectInitializer& ObjectInitializer);

	UFUNCTION(BlueprintCallable, Category = "InflectionPoint|Networking")
		void LeaveMultiplayerGame(FName SessionName);
	
private:

	/** Delegate for destroying a session */
	FOnDestroySessionCompleteDelegate OnDestroySessionCompleteDelegate;

	/** Handle to registered delegate for destroying a session */
	FDelegateHandle OnDestroySessionCompleteDelegateHandle;
	
	virtual void OnDestroySessionComplete(FName SessionName, bool bWasSuccessful);
	
};
