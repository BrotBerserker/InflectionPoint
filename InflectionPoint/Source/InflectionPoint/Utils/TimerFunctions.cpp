// Fill out your copyright notice in the Description page of Project Settings.

#include "InflectionPoint.h"
#include "TimerFunctions.h"

//template <typename UObjectTemplate, typename... VarTypes>
//static void TimerFunctions::StartTimer(UObjectTemplate* inUserObject, UWorld * world, FString function, float wait, VarTypes... vars) {
//	FTimerHandle TimerHandle;
//	FTimerDelegate TimerDel;
//	TimerDel.BindUFunction(inUserObject, FName(*function), std::forward<VarTypes>(vars)...);
//	world->GetTimerManager().SetTimer(TimerHandle, TimerDel, wait, false, wait);
//}
//
//template <typename UObjectTemplate>
//static void TimerFunctions::StartTimer(UObjectTemplate* inUserObject, UWorld * world, FString function, float wait) {
//	FTimerHandle TimerHandle;
//	FTimerDelegate TimerDel;
//	TimerDel.BindUFunction(inUserObject, FName(*function));
//	world->GetTimerManager().SetTimer(TimerHandle, TimerDel, wait, false, wait);
//}

TimerFunctions::TimerFunctions() {
}

TimerFunctions::~TimerFunctions() {
}
