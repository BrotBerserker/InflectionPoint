// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Gameplay/CharacterInfoProvider.h"
#include "Gameplay/Characters/BaseCharacter.h"
#include "Gameplay/Characters/PlayerCharacterBase.h"
#include "Gameplay/Characters/ReplayCharacterBase.h"
#include "CharacterHeadDisplayBase.generated.h"

/**
 *
 */
UCLASS()
class INFLECTIONPOINT_API UCharacterHeadDisplayBase : public UUserWidget {
	GENERATED_BODY()

public:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
		UCharacterInfoProvider* CharacterInfoProvider;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
		ABaseCharacter* Character;

	UFUNCTION(BlueprintCallable)
		FORCEINLINE class APlayerCharacterBase* GetCharacterAsPlayer() const { return Cast<APlayerCharacterBase>(Character); };
	UFUNCTION(BlueprintCallable)
		FORCEINLINE class AReplayCharacterBase* GetCharacterAsReplay() const { return Cast<AReplayCharacterBase>(Character); };
};
