// Fill out your copyright notice in the Description page of Project Settings.

#pragma once
#include "Engine.h"

template <typename UObjectTemplate, typename... VarTypes>
void StartTimer(UObjectTemplate* inUserObject, UWorld * world, FString function, float wait, VarTypes... vars) {
	FTimerHandle TimerHandle;
	FTimerDelegate TimerDel;
	TimerDel.BindUFunction(inUserObject, FName(*function), std::forward<VarTypes>(vars)...);
	world->GetTimerManager().SetTimer(TimerHandle, TimerDel, wait, false, wait);
}

/**
 *
 */
class INFLECTIONPOINT_API TimerFunctions {
public:
	TimerFunctions();
	~TimerFunctions();
};
