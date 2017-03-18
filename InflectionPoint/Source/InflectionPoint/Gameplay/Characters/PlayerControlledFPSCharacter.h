// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "BaseCharacter.h"
#include "PlayerControlledFPSCharacter.generated.h"

/**
 * 
 */
UCLASS()
class INFLECTIONPOINT_API APlayerControlledFPSCharacter : public ABaseCharacter
{
	GENERATED_BODY()
	
protected:
	// APawn interface
	virtual void SetupPlayerInputComponent(UInputComponent* InputComponent) override;
	// End of APawn interface
	
	void DEBUG_SpawnReplay();
};
