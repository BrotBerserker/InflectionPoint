// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "HUDElementBase.generated.h"

/**
 * 
 */
UCLASS()
class INFLECTIONPOINT_API UHUDElementBase : public UUserWidget
{
	GENERATED_BODY()
public:
	virtual void SetVisibility(ESlateVisibility InVisibility) override;
	
	UFUNCTION(BlueprintImplementableEvent)
		void OnPreVisibilityChange(ESlateVisibility NextVisibility);

};
