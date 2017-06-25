// Copyright 1998-2016 Epic Games, Inc. All Rights Reserved.
#pragma once 
#include "GameFramework/HUD.h"
#include "InflectionPointHUD.generated.h"

UCLASS()
class AInflectionPointHUD : public AHUD {
	GENERATED_BODY()

public:
	AInflectionPointHUD();

	/** Primary draw call for the HUD */
	virtual void DrawHUD() override;

};

