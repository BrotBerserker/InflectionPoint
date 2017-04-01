// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Blueprint/UserWidget.h"
#include "HostMenuBase.generated.h"

/**
 *
 */
UCLASS()
class INFLECTIONPOINT_API UHostMenuBase : public UUserWidget {
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Game")
		void HostServer(int playerAmount, bool lan, FString serverName);


};
