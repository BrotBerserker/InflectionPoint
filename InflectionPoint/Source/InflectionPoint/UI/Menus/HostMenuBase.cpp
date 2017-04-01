// Fill out your copyright notice in the Description page of Project Settings.

#include "InflectionPoint.h"
#include "HostMenuBase.h"

void UHostMenuBase::HostServer(int playerAmount, bool lan, FString serverName) {
	UE_LOG(LogTemp, Warning, TEXT("Servername: %s"), *serverName);
	
}
