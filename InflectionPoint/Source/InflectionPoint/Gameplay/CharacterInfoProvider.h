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
		int Team = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		bool IsReplay = false;

	// -1 if is no replay (to tell the diference between replays)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		int ReplayIndex = -1;

	FCharacterInfo() {
	}

	FCharacterInfo(FString playerName, int team, bool isReplay, int replayIndex = -1) {
		PlayerName = playerName;
		Team = team;
		IsReplay = isReplay;
		ReplayIndex = replayIndex;
	}

};

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class INFLECTIONPOINT_API UCharacterInfoProvider : public UActorComponent {
	GENERATED_BODY()

public:
	UCharacterInfoProvider();

	void GetLifetimeReplicatedProps(TArray<FLifetimeProperty> & OutLifetimeProps) const;

	UPROPERTY(BlueprintReadOnly, Replicated)
		APlayerState* PlayerState;

	UFUNCTION(BlueprintCallable)
		FCharacterInfo GetCharacterInfo();

	UFUNCTION(BlueprintCallable)
		bool IsAReplay();

	UFUNCTION(BlueprintCallable)
		bool IsInSameTeam(class ABaseCharacter* character);
};
