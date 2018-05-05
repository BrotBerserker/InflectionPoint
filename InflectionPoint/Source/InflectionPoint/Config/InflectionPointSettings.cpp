// Fill out your copyright notice in the Description page of Project Settings.

#include "InflectionPoint.h"
#include "InflectionPointSettings.h"



void UInflectionPointSettings::Save() {
	SaveConfig();
}

void UInflectionPointSettings::Load(UObject* WorldContextObject) {
	UGameplayStatics::SetSoundMixClassOverride(WorldContextObject, SoundMix, MasterSoundClass, MasterVolume, 1.f, 0.f);
	UGameplayStatics::SetSoundMixClassOverride(WorldContextObject, SoundMix, MusicSoundClass, MusicVolume, 1.f, 0.f);
	UGameplayStatics::SetSoundMixClassOverride(WorldContextObject, SoundMix, EffectsSoundClass, EffectsVolume, 1.f, 0.f);
	UGameplayStatics::GetPlayerController(WorldContextObject, 0)->ConsoleCommand(*FString("r.MotionBlur.Amount " + FString::SanitizeFloat(MotionBlurAmount)));
	UGameplayStatics::GetPlayerController(WorldContextObject, 0)->ConsoleCommand(*FString("r.ScreenPercentage " + FString::SanitizeFloat(ResolutionScale)));
}

void UInflectionPointSettings::SetMasterVolume(UObject* WorldContextObject, float volume) {
	MasterVolume = volume;
	UGameplayStatics::SetSoundMixClassOverride(WorldContextObject, SoundMix, MasterSoundClass, volume, 1.f, 0.f);
}

void UInflectionPointSettings::SetMusicVolume(UObject* WorldContextObject, float volume) {
	MusicVolume = volume;
	UGameplayStatics::SetSoundMixClassOverride(WorldContextObject, SoundMix, MusicSoundClass, volume, 1.f, 0.f);
}

void UInflectionPointSettings::SetEffectsVolume(UObject* WorldContextObject, float volume) {
	EffectsVolume = volume;
	UGameplayStatics::SetSoundMixClassOverride(WorldContextObject, SoundMix, EffectsSoundClass, volume, 1.f, 0.f);
}

void UInflectionPointSettings::SetMotionBlurAmount(UObject* WorldContextObject, float amount) {
	MotionBlurAmount = amount;
	UGameplayStatics::GetPlayerController(WorldContextObject, 0)->ConsoleCommand(*FString("r.MotionBlur.Amount " + FString::SanitizeFloat(amount)));
}

void UInflectionPointSettings::SetResolutionScale(UObject* WorldContextObject, float scale) {
	ResolutionScale = scale;
	UGameplayStatics::GetPlayerController(WorldContextObject, 0)->ConsoleCommand(*FString("r.ScreenPercentage " + FString::SanitizeFloat(scale)));
}
