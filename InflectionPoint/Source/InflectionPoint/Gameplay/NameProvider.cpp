// Fill out your copyright notice in the Description page of Project Settings.

#include "InflectionPoint.h"
#include "NameProvider.h"


// Sets default values for this component's properties
UNameProvider::UNameProvider() {
	PrimaryComponentTick.bCanEverTick = true;
	bReplicates = true;
}


void UNameProvider::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const {
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UNameProvider, Name);
}