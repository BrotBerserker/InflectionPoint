// Fill out your copyright notice in the Description page of Project Settings.

#include "InflectionPoint.h"
#include "TransformReplayer.h"
#include "Runtime/Engine/Classes/Components/TimelineComponent.h"
#include "Utils/CheckFunctions.h"
#include "Utils/TimerFunctions.h"

// Sets default values for this component's properties
UTransformReplayer::UTransformReplayer() {
}


// Called when the game starts
void UTransformReplayer::BeginPlay() {
	Super::BeginPlay();
	InputComponent = UGameplayStatics::GetPlayerCharacter(GetWorld(), 0)->InputComponent;

	if(!PositionRecorder)
		PositionRecorder = GetOwner()->FindComponentByClass<UTransformRecorder>(); 
	AssertNotNull(PositionRecorder , GetWorld(), __FILE__, __LINE__);

	if(!InputComponent)
		return;
	InputComponent->BindAction("DEBUG_SpawnReplay", IE_Pressed, this, &UTransformReplayer::PLayReplay); 
	AssertNotNull(InputComponent , GetWorld(), __FILE__, __LINE__);
}

// Called when the game starts
void UTransformReplayer::PLayReplay() {
	TArray<FTimeStamp> record = PositionRecorder->StopRecording();
	if(record.Num() < 1)
		return;
	AActor* obj = PositionRecorder->GetOwner();
	obj->SetActorLocation(record[0].Location);
	obj->SetActorRotation(record[0].Rotation);

	for(int i = 1; i < record.Num(); i++) {
		FTimeStamp& aStamp = record[i - 1]; // first
		FTimeStamp& bStamp = record[i]; // second

		// create timer
		StartTimer(this, GetWorld(), "PerformMovingStep", aStamp.TimeSeconds, aStamp, bStamp);
	}
}


void UTransformReplayer::PerformMovingStep(FTimeStamp aStamp, FTimeStamp bStamp) {
	float timeDelta = bStamp.TimeSeconds - aStamp.TimeSeconds;
	FLatentActionInfo latentInfo;
	latentInfo.CallbackTarget = this;
	UKismetSystemLibrary::MoveComponentTo(PositionRecorder->GetOwner()->GetRootComponent(), bStamp.Location, bStamp.Rotation.Rotator(), false, false, timeDelta, true, EMoveComponentAction::Type::Move, latentInfo);
}