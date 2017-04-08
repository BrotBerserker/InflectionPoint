// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Blueprint/UserWidget.h"
#include "SubMenuTemplate.generated.h"

/**
 *
 */
UCLASS(abstract)
class INFLECTIONPOINT_API USubMenuTemplate : public UUserWidget {
	GENERATED_BODY()
public:
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Menu")
		UUserWidget* ParentWidgetMenu;

	UFUNCTION(BlueprintCallable, Category = "Menu")
		void NavigateBackToParentWidgetMenu();
};
