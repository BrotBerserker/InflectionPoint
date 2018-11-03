// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"


template <typename F>
void DoShitForAllPlayerControllers(UWorld* world, F shit) {
	for(FConstPlayerControllerIterator Iterator = world->GetPlayerControllerIterator(); Iterator; ++Iterator) {
		auto playerController = UGameplayStatics::GetPlayerController(world, Iterator.GetIndex());
		auto ipPlayerController = Cast<APlayerControllerBase>(playerController);
		shit(ipPlayerController);
	}
}

/**
 * 
 */
class INFLECTIONPOINT_API ChickenFunctions
{
public:
	ChickenFunctions();
	~ChickenFunctions();
};
