// Fill out your copyright notice in the Description page of Project Settings.

#pragma once
#include "Engine.h"

bool AssertNotNull(std::initializer_list<void*> pointers, UWorld* world, FString file, int line, FString description = "No description provided");

bool AssertNotNull(void* pointer, UWorld* world, FString file, int line, FString description = "No description provided");

/**
 *
 */
class INFLECTIONPOINT_API CheckFunctions {
public:
	CheckFunctions();
	~CheckFunctions();
};
