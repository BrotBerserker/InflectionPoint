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
		bool IsReplay = false;

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

	void GetLifetimeReplicatedProps(TArray<FLifetimeProperty> & OutLifetimeProps) const;


	UPROPERTY(BlueprintReadOnly, Replicated)
		bool IsReplay = false;

	UPROPERTY(BlueprintReadOnly, Replicated)
		APlayerState* PlayerState;

	UFUNCTION(BlueprintCallable)
		FCharacterInfo GetCharacterInfo();
};
