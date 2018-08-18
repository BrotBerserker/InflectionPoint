// Fill out your copyright notice in the Description page of Project Settings.

#include "InflectionPoint.h"
#include "CheckFunctions.h"
#include <typeinfo>

bool AssertNotNull(void* pointer, UWorld* world, FString file, int line, FString description) {
	if(pointer == nullptr) {
		UE_LOG(LogTemp, Error, TEXT("Nullptr in file: %s at line: %d [%s]"), *file, line, *description);
		GEngine->AddOnScreenDebugMessage(-1, 5, FColor::Red, TEXT("ALARM! Nullptr spotted!!"));
		UGameplayStatics::SetGamePaused(world, true);
		return false;
	}
	return true;
}

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

bool AssertTrue(bool check, UWorld* world, FString file, int line, FString description) {
	if(check)
		return true;
	UE_LOG(LogTemp, Error, TEXT("Failed Assert in file: %s at line: %d [%s]"), *file, line, *description);
	GEngine->AddOnScreenDebugMessage(-1, 5, FColor::Red, TEXT("ALARM! Assert failed!!"));
	UGameplayStatics::SetGamePaused(world, true);
	return false;
}

bool SoftAssertTrue(bool check, UWorld* world, FString file, int line, FString description) {
	if(check)
		return true;
	UE_LOG(LogTemp, Error, TEXT("Failed SoftAssert in file: %s at line: %d [%s]"), *file, line, *description);
	GEngine->AddOnScreenDebugMessage(-1, 15, FColor::Orange, FString::Printf(TEXT("SoftAssert failed! [%s]"), *description));
	return false;
}


void DebugPrint(FString file, int line, FString description) {
	UE_LOG(LogTemp, Warning, TEXT("DebugPrint in file: %s at line: %d [%s]"), *file, line, *description);
}

CheckFunctions::CheckFunctions() {}

CheckFunctions::~CheckFunctions() {}
