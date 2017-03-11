// Fill out your copyright notice in the Description page of Project Settings.

#include "InflectionPoint.h"
#include "TestRecordPlayer.h"
#include "Runtime/Engine/Classes/Components/TimelineComponent.h"


// Sets default values for this component's properties
UTestRecordPlayer::UTestRecordPlayer()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}


// Called when the game starts
void UTestRecordPlayer::BeginPlay()
{
	Super::BeginPlay();
	InputComponent = UGameplayStatics::GetPlayerCharacter(GetWorld(), 0)->InputComponent;

	if (!PositionRecorder) {
		PositionRecorder = GetOwner()->FindComponentByClass<UPositionRecorder>();
	}

	if (!InputComponent)
		return;
	InputComponent->BindAction( "DEBUG_SpawnReplay", IE_Pressed, this, &UTestRecordPlayer::PLayReplay);	
}


// Called every frame
void UTestRecordPlayer::TickComponent( float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction )
{
	Super::TickComponent( DeltaTime, TickType, ThisTickFunction );

	// ...
}



// Called when the game starts
void UTestRecordPlayer::PLayReplay() {
	UE_LOG(LogTemp, Warning, TEXT("Play Replay"));
	TArray<FTimeStamp> record = PositionRecorder->StopRecording();
	if (record.Num() < 1)
		return;
	AActor* obj = PositionRecorder->GetOwner();
	obj->SetActorLocation(record[0].Location);
	obj->SetActorRotation(record[0].Rotation);
	// print array
	//for (int i = 0; i < record.Num(); i++) {
	//	FTimeStamp& aStamp = record[i];
	//	UE_LOG(LogTemp, Warning, TEXT("Element [%i] time: %f rotation: %s location: %s"), i, aStamp.TimeSeconds, *aStamp.Location.ToString(), *aStamp.Rotation.ToString());
	//}

	for (int i = 1; i < record.Num(); i++) {
		FTimeStamp& aStamp = record[i-1];
		FTimeStamp& bStamp = record[i];
		//UE_LOG(LogTemp, Warning, TEXT("Stemp at %f sec"), aStamp.TimeSeconds);

		FTimerHandle TimerHandle;
		FTimerDelegate TimerDel;
		TimerDel.BindUFunction(this, FName("PerformMovingStep"), aStamp, bStamp);
		GetWorld()->GetTimerManager().SetTimer(TimerHandle, TimerDel, aStamp.TimeSeconds, false, aStamp.TimeSeconds);
	}
}


void UTestRecordPlayer::PerformMovingStep(FTimeStamp aStamp, FTimeStamp bStamp) {	
	float timeDelta = bStamp.TimeSeconds - aStamp.TimeSeconds;
	FLatentActionInfo latentInfo;
	latentInfo.CallbackTarget = this;
	UKismetSystemLibrary::MoveComponentTo(PositionRecorder->GetOwner()->GetRootComponent(), bStamp.Location, bStamp.Rotation.Rotator(), false, false, timeDelta, true, EMoveComponentAction::Type::Move, latentInfo);
	

	//FTimeline timeline = FTimeline();
	//FOnTimelineFloat progressFunction{};
	//progressFunction.BindUFunction(this, FName("EffectProgress"));

	//auto richCurve = new FRichCurve();
	//richCurve->AddKey(0.f, 0.f);
	//richCurve->AddKey(timeDelta, 1.f);

	//auto curve = NewObject<UCurveFloat>();
	//curve->GetCurves().Add(FRichCurveEditInfo(richCurve, FName{ TEXT("LinearCurve") }));
	//timeline.AddInterpFloat(curve, progressFunction, FName{ TEXT("EFFECT_TIMED") });
	//
	//// Start
	//timeline.PlayFromStart();
}