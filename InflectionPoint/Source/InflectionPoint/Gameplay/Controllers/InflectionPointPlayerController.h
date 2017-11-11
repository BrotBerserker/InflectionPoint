// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/PlayerController.h"
#include "Gameplay/CharacterInfoProvider.h"
#include "InflectionPointPlayerController.generated.h"

/**
 *
 */
UCLASS()
class INFLECTIONPOINT_API AInflectionPointPlayerController : public APlayerController {
	GENERATED_BODY()
public:
	AInflectionPointPlayerController(const FObjectInitializer& ObjectInitializer);

	void Possess(APawn* InPawn) override;

	UFUNCTION(Client, Reliable)
		void ClientSetControlRotation(FRotator rotation);

	UPROPERTY()
		class UCharacterInfoProvider* CharacterInfoProvider;

};
