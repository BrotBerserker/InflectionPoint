// Fill out your copyright notice in the Description page of Project Settings.

#include "InflectionPoint.h"
#include "SubMenuTemplate.h"

void USubMenuTemplate::NavigateBackToParentWidgetMenu() {
	if(!AssertNotNull(ParentWidgetMenu, GetWorld(), __FILE__, __LINE__))
		return;
	this->SetVisibility(ESlateVisibility::Hidden);
	ParentWidgetMenu->SetVisibility(ESlateVisibility::Visible);
}
