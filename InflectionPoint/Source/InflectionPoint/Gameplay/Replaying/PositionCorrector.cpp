// Fill out your copyright notice in the Description page of Project Settings.

#include "InflectionPoint.h"
#include "PositionCorrector.h"
#include "DrawDebugHelpers.h"


// Sets default values for this component's properties
UPositionCorrector::UPositionCorrector() {
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}


// Called when the game starts
void UPositionCorrector::BeginPlay() {
	Super::BeginPlay();

}


// Called every frame
void UPositionCorrector::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) {
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

void UPositionCorrector::StartCorrecting(TArray<TPair<float, FVector>> positions) {
	while(positions.Num() > 0) {
		TPair<float, FVector> pair = positions.Last();
		positions.RemoveAt(positions.Num() - 1);

		//UE_LOG(LogTemp, Warning, TEXT("Waiting %f and then pressing %f!"), wait, key);

		FTimerHandle TimerHandle;
		FTimerDelegate TimerDel;
		TimerDel.BindUFunction(this, FName("CorrectPosition"), pair.Value);

		GetWorld()->GetTimerManager().SetTimer(TimerHandle, TimerDel, pair.Key, false, pair.Key);
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

