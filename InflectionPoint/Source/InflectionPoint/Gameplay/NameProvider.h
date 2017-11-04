// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "NameProvider.generated.h"


UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class INFLECTIONPOINT_API UNameProvider : public UActorComponent {
	GENERATED_BODY()

public:
	UNameProvider();

	UPROPERTY(Replicated, BlueprintReadOnly)
		FString Name;

	/** Needed for replication of Name */
	void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
};
