// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Gameplay/CharacterInfoProvider.h"
#include "Gameplay/Characters/BaseCharacter.h"
#include "Gameplay/Characters/PlayerCharacterBase.h"
#include "Gameplay/Characters/ReplayCharacterBase.h"
#include "UI/HUD/HUDElementBase.h"
#include "CharacterHeadDisplayBase.generated.h"

/**
 *
 */
UCLASS()
class INFLECTIONPOINT_API UCharacterHeadDisplayBase : public UHUDElementBase {
	GENERATED_BODY()

public:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
		ABaseCharacter* OwningCharacter;
};
