// Fill out your copyright notice in the Description page of Project Settings.

#include "InflectionPoint.h"
#include "Countdown.h"
#include "Engine.h"


// Sets default values for this component's properties
UCountdown::UCountdown() {
	PrimaryComponentTick.bCanEverTick = true;
}

void UCountdown::BeginPlay() {
	Super::BeginPlay();
}

void UCountdown::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) {
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if(RemainingSeconds < 0) {
		return;
	}

	TimeUntilNextUpdate -= DeltaTime;

	if(TimeUntilNextUpdate <= 0.f) {
		TimeUntilNextUpdate = 1.f;
		OnUpdate.ExecuteIfBound(RemainingSeconds--);
		if(RemainingSeconds == -1) {
			OnFinish.ExecuteIfBound();
		}
	}
}

void UCountdown::Start() {
	RemainingSeconds = Duration;
	TimeUntilNextUpdate = 0.f;
}

void UCountdown::Stop() {
	RemainingSeconds = -1;
	TimeUntilNextUpdate = 1.f;
}
