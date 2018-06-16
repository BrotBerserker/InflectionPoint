// Fill out your copyright notice in the Description page of Project Settings.

#include "InflectionPoint.h"
#include "InflectionPointGameInstanceBase.h"


UInflectionPointGameInstanceBase::UInflectionPointGameInstanceBase() {
	IPSettings = CreateDefaultSubobject<UInflectionPointSettings>(TEXT("IPSettings"));
}

void UInflectionPointGameInstanceBase::Init() {
	Super::Init();

	FCoreUObjectDelegates::PreLoadMap.AddUObject(this, &UInflectionPointGameInstanceBase::BeginLoadingScreen);
	FCoreUObjectDelegates::PostLoadMapWithWorld.AddUObject(this, &UInflectionPointGameInstanceBase::EndLoadingScreen);
}

void UInflectionPointGameInstanceBase::BeginLoadingScreen(const FString& InMapName) {
	UE_LOG(LogTemp, Warning, TEXT("BeginLoadingScreen"));

	if(!IsRunningDedicatedServer()) {
		FLoadingScreenAttributes LoadingScreen;
		LoadingScreen.bAutoCompleteWhenLoadingCompletes = false;
		auto loadingWidget = CreateWidget<UUserWidget>(this, WidgetLoadingScreen);
		
		if(SoftAssertTrue(loadingWidget != nullptr, GetWorld(), __FILE__, __LINE__,"No Loading Screen Widget"))
			LoadingScreen.WidgetLoadingScreen = loadingWidget->TakeWidget();

		GetMoviePlayer()->SetupLoadingScreen(LoadingScreen);
	}
}

void UInflectionPointGameInstanceBase::EndLoadingScreen(UWorld* InLoadedWorld) {
	UE_LOG(LogTemp, Warning, TEXT("EndLoadingScreen"));
}