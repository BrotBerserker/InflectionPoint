// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "CharacterInfoProvider.generated.h"

USTRUCT(BlueprintType)
struct FCharacterInfo {
	GENERATED_USTRUCT_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString PlayerName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		int Team;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		bool IsReplay;

	FCharacterInfo() {
	}

	FCharacterInfo(FString playerName, int team, bool isReplay) {
		PlayerName = playerName;
		Team = team;
		IsReplay = isReplay;
	}

};

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class INFLECTIONPOINT_API UCharacterInfoProvider : public UActorComponent {
	GENERATED_BODY()

public:
	UCharacterInfoProvider();

	APlayerState* PlayerState;

	bool IsReplay = false;

	FCharacterInfo GetCharacterInfo();
};
