// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "SubMenuTemplate.h"
#include "Blueprint/UserWidget.h"
#include "HostMenuBase.generated.h"

/**
 *
 */
UCLASS()
class INFLECTIONPOINT_API UHostMenuBase : public USubMenuTemplate {
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Game")
		void HostServer(int playerAmount, bool lan, FString serverName);
};
