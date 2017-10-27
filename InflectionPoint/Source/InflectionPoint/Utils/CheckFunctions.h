// Fill out your copyright notice in the Description page of Project Settings.

#pragma once
#include "Engine.h"
bool AssertNotNull(std::initializer_list<void*> pointers, UWorld* world, FString file, int line, FString description = "No description provided");

bool AssertNotNull(void* pointer, UWorld* world, FString file, int line, FString description = "No description provided");

bool AssertTrue(bool check, UWorld* world, FString file, int line, FString description = "No description provided"); 
bool SoftAssertTrue(bool check, UWorld* world, FString file, int line, FString description = "No description provided");

//template<typename T>
//bool AssertCanCast(void* type, UWorld* world, FString file, int line, FString description = "No description provided") {
//	if(typeid(*type) == typeid(T))
//		return true;
//	UE_LOG(LogTemp, Error, TEXT("Failed cast in file: %s at line: %d [%s]"), *file, line, *description);
//	GEngine->AddOnScreenDebugMessage(-1, 5, FColor::Red, TEXT("ALARM! Cast fail spotted!!"));
//	UGameplayStatics::SetGamePaused(world, true);
//	return false;
//}


/**
 *
 */
class INFLECTIONPOINT_API CheckFunctions {
public:
	CheckFunctions();
	~CheckFunctions();
};
