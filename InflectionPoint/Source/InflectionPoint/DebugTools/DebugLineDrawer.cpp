// Fill out your copyright notice in the Description page of Project Settings.

#include "InflectionPoint.h"
#include "DebugTools/InflectionPointCheatManager.h"
#include "Gameplay/Characters/ReplayCharacterBase.h"
#include "DebugLineDrawer.h"


// Sets default values for this component's properties
UDebugLineDrawer::UDebugLineDrawer() {
	PrimaryComponentTick.bCanEverTick = false;
}

// Called when the game starts
void UDebugLineDrawer::BeginPlay() {
	Super::BeginPlay();
}

// Called every frame
void UDebugLineDrawer::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) {
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

void UDebugLineDrawer::DrawDebugLineTrace(FVector StartPos, FVector EndPos) {
	if(GetOwner()->Instigator) {
		DrawDebugLineTrace(StartPos, EndPos, GetOwner()->Instigator->IsA(AReplayCharacterBase::StaticClass()));
	} else {
		DrawDebugLineTrace(StartPos, EndPos, false);
	}
}

void UDebugLineDrawer::DrawPlayerDebugLineTrace(FVector StartPos, FVector EndPos) {
	DrawDebugLineTrace(StartPos, EndPos, false);
}

void UDebugLineDrawer::DrawReplayDebugLineTrace(FVector StartPos, FVector EndPos) {
	DrawDebugLineTrace(StartPos, EndPos, true);
}

void UDebugLineDrawer::DrawDebugLineTrace(FVector StartPos, FVector EndPos, bool IsReplay) {
	auto cheatManager = Cast<UInflectionPointCheatManager>(GetWorld()->GetFirstPlayerController()->CheatManager);
	if(!(cheatManager && cheatManager->IsDebugProjectileLineTraceEnabled))
		return;
	if(!IsReplay) {
		DrawDebugLine(GetWorld(), StartPos, EndPos, PlayerDebugColor, true, -1, 0, LineThickness);
		DrawDebugPoint(GetWorld(), EndPos, 3, PlayerDebugColor, true);
	} else {
		DrawDebugLine(GetWorld(), StartPos, EndPos, ReplayDebugColor, true, -1, 0, LineThickness);
		DrawDebugPoint(GetWorld(), EndPos, 3, ReplayDebugColor, true);
	}
}