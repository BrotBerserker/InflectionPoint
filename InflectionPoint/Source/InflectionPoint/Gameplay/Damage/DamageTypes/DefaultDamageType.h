// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/DamageType.h"
#include "DefaultDamageType.generated.h"

/**
 *
 */
UCLASS()
class INFLECTIONPOINT_API UDefaultDamageType : public UDamageType {
	GENERATED_BODY()

public:
	/** Sound to play when damaged by this damage type */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Effects)
		USoundCue* DamageSound;
};
