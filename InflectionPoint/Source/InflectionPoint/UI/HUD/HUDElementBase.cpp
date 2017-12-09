// Fill out your copyright notice in the Description page of Project Settings.

#include "InflectionPoint.h"
#include "HUDElementBase.h"



void UHUDElementBase::SetVisibility(ESlateVisibility InVisibility) {
	OnPreVisibilityChange(InVisibility);
	Super::SetVisibility(InVisibility);
}
