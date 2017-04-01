// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Blueprint/UserWidget.h"
#include "HostMenu.generated.h"

/**
 *
 */
UCLASS()
class INFLECTIONPOINT_API UHostMenu : public UUserWidget {
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Game")
		void DiesDasAnanas();


};
