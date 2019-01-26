// Fill out your copyright notice in the Description page of Project Settings.

#include "InflectionPoint.h"
#include "Countdown.h"
#include "Engine.h"


// Sets default values for this component's properties
UCountdown::UCountdown() {DebugPrint(__FILE__, __LINE__);
	PrimaryComponentTick.bCanEverTick = true;
DebugPrint(__FILE__, __LINE__);}

void UCountdown::BeginPlay() {DebugPrint(__FILE__, __LINE__);
	Super::BeginPlay();
DebugPrint(__FILE__, __LINE__);}

void UCountdown::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) {DebugPrint(__FILE__, __LINE__);
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if(RemainingSeconds < 0) {DebugPrint(__FILE__, __LINE__);
		return;
	DebugPrint(__FILE__, __LINE__);}

	TimeUntilNextUpdate -= DeltaTime;

	if(TimeUntilNextUpdate <= 0.f) {DebugPrint(__FILE__, __LINE__);
		TimeUntilNextUpdate = 1.f;
		OnUpdate.ExecuteIfBound(RemainingSeconds--);
		if(RemainingSeconds == -1) {DebugPrint(__FILE__, __LINE__);
			OnFinish.ExecuteIfBound();
		DebugPrint(__FILE__, __LINE__);}
	DebugPrint(__FILE__, __LINE__);}
DebugPrint(__FILE__, __LINE__);}

void UCountdown::Start() {DebugPrint(__FILE__, __LINE__);
	RemainingSeconds = Duration;
	TimeUntilNextUpdate = InstantUpdate ? 0.f : 1.f;
DebugPrint(__FILE__, __LINE__);}

void UCountdown::Stop() {DebugPrint(__FILE__, __LINE__);
	RemainingSeconds = -1;
	TimeUntilNextUpdate = 1.f;
DebugPrint(__FILE__, __LINE__);}
