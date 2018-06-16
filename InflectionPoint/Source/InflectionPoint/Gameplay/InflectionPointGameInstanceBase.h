// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Config/InflectionPointSettings.h"
#include "Engine/GameInstance.h"
#include "Runtime/MoviePlayer/Public/MoviePlayer.h"
#include "InflectionPointGameInstanceBase.generated.h"

/**
 *
 */
UCLASS()
class INFLECTIONPOINT_API UInflectionPointGameInstanceBase : public UGameInstance {
	GENERATED_BODY()

public:
	UInflectionPointGameInstanceBase();

	virtual void Init() override;

	UFUNCTION()
		virtual void BeginLoadingScreen(const FString& MapName);
	UFUNCTION()
		virtual void EndLoadingScreen(UWorld* InLoadedWorld);
public:
	UPROPERTY(BlueprintReadWrite)
		FName CurrentSessionName;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
		class UInflectionPointSettings* IPSettings;


	UPROPERTY(BlueprintReadWrite, EditAnywhere)
		TSubclassOf<class UUserWidget> WidgetLoadingScreen;
};
