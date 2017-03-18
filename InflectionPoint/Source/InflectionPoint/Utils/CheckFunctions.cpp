// Fill out your copyright notice in the Description page of Project Settings.

#include "InflectionPoint.h"
#include "CheckFunctions.h"
#include <typeinfo>

bool AssertNotNull(std::initializer_list<void*> pointers, UWorld* world, FString file, int line, FString description) {
	void * const * current = pointers.begin();
	while(current != pointers.end()) {
		if(!AssertNotNull(*current, world, file, line, description)) {
			return false;
		}
		current++;
	} 
	return true;
}

bool AssertNotNull(void* pointer, UWorld* world, FString file, int line, FString description) {
	if(pointer == nullptr) {
		UE_LOG(LogTemp, Error, TEXT("Nullptr in file: %s at line: %d [%s]"), *file, line, *description);
		GEngine->AddOnScreenDebugMessage(-1, 5, FColor::Red, TEXT("ALARM! Nullptr spotted!!"));
		UGameplayStatics::SetGamePaused(world, true);
		return false;
	}
	return true;
}

CheckFunctions::CheckFunctions()
{
}

CheckFunctions::~CheckFunctions()
{
}
