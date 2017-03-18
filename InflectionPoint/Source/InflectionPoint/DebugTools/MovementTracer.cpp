// Fill out your copyright notice in the Description page of Project Settings.

#include "InflectionPoint.h"
#include "MovementTracer.h"
#include "DrawDebugHelpers.h"


// Sets default values for this component's properties
UMovementTracer::UMovementTracer()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}


// Called when the game starts
void UMovementTracer::BeginPlay()
{
	Super::BeginPlay();

	// ...
	
}


// Called every frame
void UMovementTracer::TickComponent( float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction )
{
	Super::TickComponent( DeltaTime, TickType, ThisTickFunction );
	if(Enabled) {
		DrawDebugPoint(GetWorld(), GetOwner()->GetTransform().GetLocation(), 10, LineColor, true);
	}
}

