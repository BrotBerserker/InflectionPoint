// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "InflectionPointSettings.generated.h"

/**
 *
 */
UCLASS(Config = Game, ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class INFLECTIONPOINT_API UInflectionPointSettings : public UActorComponent {
	GENERATED_BODY()

public:
	UPROPERTY(Config, BlueprintReadOnly)
		float MasterVolume;

	UPROPERTY(Config, BlueprintReadOnly)
		float MusicVolume;

	UPROPERTY(Config, BlueprintReadOnly)
		float EffectsVolume;

	UPROPERTY(Config, BlueprintReadOnly)
		float MotionBlurAmount;

	UPROPERTY(Config, BlueprintReadOnly)
		float ResolutionScale;

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
		USoundMix* SoundMix;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
		USoundClass* MasterSoundClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
		USoundClass* MusicSoundClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
		USoundClass* EffectsSoundClass;

public:
	UFUNCTION(BlueprintCallable, meta = (WorldContext = "WorldContextObject"))
		void SetMasterVolume(UObject* WorldContextObject, float volume);

	UFUNCTION(BlueprintCallable, meta = (WorldContext = "WorldContextObject"))
		void SetMusicVolume(UObject* WorldContextObject, float volume);

	UFUNCTION(BlueprintCallable, meta = (WorldContext = "WorldContextObject"))
		void SetEffectsVolume(UObject* WorldContextObject, float volume);

	UFUNCTION(BlueprintCallable, meta = (WorldContext = "WorldContextObject"))
		void SetMotionBlurAmount(UObject* WorldContextObject, float amount);

	UFUNCTION(BlueprintCallable, meta = (WorldContext = "WorldContextObject"))
		void SetResolutionScale(UObject* WorldContextObject, float scale);

public:
	UFUNCTION(BlueprintCallable)
		void Save();

	UFUNCTION(BlueprintCallable, meta = (WorldContext = "WorldContextObject"))
		void Load(UObject* WorldContextObject);
};
