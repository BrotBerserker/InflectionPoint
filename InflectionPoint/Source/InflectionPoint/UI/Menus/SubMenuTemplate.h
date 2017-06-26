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
	/* ---------------------- */
	/*   Blueprint Settings   */
	/* ---------------------- */
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Menu")
		UUserWidget* ParentWidgetMenu;

public:
	/* ------------- */
	/*   Functions   */
	/* ------------- */

	UFUNCTION(BlueprintCallable, Category = "Menu")
		void NavigateBackToParentWidgetMenu();
};
