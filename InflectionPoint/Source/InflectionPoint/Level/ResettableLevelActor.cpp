// Fill out your copyright notice in the Description page of Project Settings.

#include "InflectionPoint.h"
#include "ResettableLevelActor.h"


// Sets default values
AResettableLevelActor::AResettableLevelActor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void AResettableLevelActor::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AResettableLevelActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AResettableLevelActor::ResetLevelActor() {
	OnLevelActorReset();
}
