// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "HUDElementBase.generated.h"


UENUM(Blueprintable, Meta = (Bitflags))
enum class EHUDElementView : uint8 {
	EmptyView,
	IngameView,
	CountdownView,
	DeathView,
	SpectatorView,
	MatchEndView,
}; 
ENUM_CLASS_FLAGS(EHUDElementView);

/**
 * 
 */
UCLASS()
class INFLECTIONPOINT_API UHUDElementBase : public UUserWidget
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere, Category = HUD, meta = (Bitmask, BitmaskEnum = "EHUDElementView"))
		uint8 HUDViewAffiliation;

	virtual void SetVisibility(ESlateVisibility InVisibility) override;
	
	UFUNCTION(BlueprintImplementableEvent)
		void OnPreVisibilityChange(ESlateVisibility NextVisibility);

	UFUNCTION(BlueprintCallable)
		void SwitchHUDView(EHUDElementView view);
};
