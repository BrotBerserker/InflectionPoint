// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/PlayerController.h"
#include "Gameplay/CharacterInfoProvider.h"
#include "PlayerControllerBase.generated.h"

/**
 *
 */
UCLASS()
class INFLECTIONPOINT_API APlayerControllerBase : public APlayerController {
	GENERATED_BODY()
public:
	APlayerControllerBase(const FObjectInitializer& ObjectInitializer);

	void BeginPlay() override;

	void Possess(APawn* InPawn) override;

	UFUNCTION(Client, Reliable)
		void ClientSetControlRotation(FRotator rotation);

	UPROPERTY()
		class UCharacterInfoProvider* CharacterInfoProvider;

};
