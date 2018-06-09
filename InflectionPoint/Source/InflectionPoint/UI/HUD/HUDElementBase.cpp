// Fill out your copyright notice in the Description page of Project Settings.

#include "InflectionPoint.h"
#include "HUDElementBase.h"



void UHUDElementBase::SetVisibility(ESlateVisibility InVisibility) {
	OnPreVisibilityChange(InVisibility);
	Super::SetVisibility(InVisibility);
}

void UHUDElementBase::SwitchHUDView(EHUDElementView view) {
	bool shouldBeVisible = ((HUDViewAffiliation) & (1 << static_cast<uint32>(view))) > 0;	
	SetVisibility(shouldBeVisible ? ESlateVisibility::Visible : ESlateVisibility::Hidden);
}