// Fill out your copyright notice in the Description page of Project Settings.

#include "InflectionPoint.h"
#include "SubMenuTemplate.h"
#include "Utils/CheckFunctions.h"

void USubMenuTemplate::NavigateBackToParentWidgetMenu() {
	if(!AssertNotNull(ParentWidgetMenu, GetWorld(), __FILE__, __LINE__))
		return;
	this->RemoveFromViewport();
	ParentWidgetMenu->AddToViewport();
}
