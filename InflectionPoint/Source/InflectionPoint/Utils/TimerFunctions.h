// Fill out your copyright notice in the Description page of Project Settings.

#pragma once
#include "Engine.h"

/**
 *
 */
class INFLECTIONPOINT_API TimerFunctions {
public:

	template <typename UObjectTemplate, typename... VarTypes>
	static void TimerFunctions::StartTimer(UObjectTemplate* inUserObject, UWorld * world, FString function, float wait, VarTypes... vars) {
		FTimerHandle TimerHandle;
		FTimerDelegate TimerDel;
		TimerDel.BindUFunction(inUserObject, FName(*function), std::forward<VarTypes>(vars)...);
		world->GetTimerManager().SetTimer(TimerHandle, TimerDel, wait, false, wait);
	}

	TimerFunctions();
	~TimerFunctions();
};
