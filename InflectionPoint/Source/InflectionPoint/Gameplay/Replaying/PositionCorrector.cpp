// Fill out your copyright notice in the Description page of Project Settings.

#include "InflectionPoint.h"
#include "PositionCorrector.h"
#include "DrawDebugHelpers.h"
#include "Utils/TimerFunctions.h"

// Sets default values for this component's properties
UPositionCorrector::UPositionCorrector() { }


// Called when the game starts
void UPositionCorrector::BeginPlay() {
	Super::BeginPlay();
}

void UPositionCorrector::StartCorrecting(TArray<TPair<float, FVector>> positions) {
	while(positions.Num() > 0) {
		TPair<float, FVector> pair = positions.Last();
		positions.RemoveAt(positions.Num() - 1);

		StartTimer(this, GetWorld(), "CorrectPosition", pair.Key, pair.Value);
	}
}

void UPositionCorrector::CorrectPosition(FVector correctPosition) {	
	FVector actualPosition = GetOwner()->GetTransform().GetLocation();
	
	if(CorrectionRadius < 0 || FVector::Dist(actualPosition, correctPosition) <= CorrectionRadius) {
		GetOwner()->SetActorLocation(correctPosition);
		if(CreateDebugCorrectionSpheres) {
			DrawDebugSphere(GetWorld(), actualPosition, CorrectionRadius, 8, DebugHitColor, true);
		}
	} else {
		if(CreateDebugCorrectionSpheres) {
			DrawDebugSphere(GetWorld(), actualPosition, CorrectionRadius, 8, DebugMissColor, true);
		}
	}
}

