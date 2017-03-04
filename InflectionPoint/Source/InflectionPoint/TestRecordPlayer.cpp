// Fill out your copyright notice in the Description page of Project Settings.

#include "InflectionPoint.h"
#include "TestRecordPlayer.h"


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
	InputComponent->BindAction( "Jump", IE_Pressed, this, &UTestRecordPlayer::PLayReplay);	
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
	// TODO: move the object based on record (interpolation)
	for (auto& var : record) {
		UE_LOG(LogTemp, Warning, TEXT("Stemp at %f sec"), var.TimeSeconds);
	}
}

