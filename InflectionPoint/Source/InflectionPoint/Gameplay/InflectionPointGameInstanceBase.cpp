// Fill out your copyright notice in the Description page of Project Settings.

#include "InflectionPoint.h"
#include "InflectionPointGameInstanceBase.h"


UInflectionPointGameInstanceBase::UInflectionPointGameInstanceBase() {DebugPrint(__FILE__, __LINE__);
	IPSettings = CreateDefaultSubobject<UInflectionPointSettings>(TEXT("IPSettings"));
}

void UInflectionPointGameInstanceBase::Init() {DebugPrint(__FILE__, __LINE__);
	Super::Init();

	FCoreUObjectDelegates::PreLoadMap.AddUObject(this, &UInflectionPointGameInstanceBase::BeginLoadingScreen);
	FCoreUObjectDelegates::PostLoadMapWithWorld.AddUObject(this, &UInflectionPointGameInstanceBase::EndLoadingScreen);
}

void UInflectionPointGameInstanceBase::BeginLoadingScreen(const FString& InMapName) {DebugPrint(__FILE__, __LINE__);
	UE_LOG(LogTemp, Warning, TEXT("BeginLoadingScreen"));

	if(!IsRunningDedicatedServer()) {DebugPrint(__FILE__, __LINE__);
		FLoadingScreenAttributes LoadingScreen;
		LoadingScreen.bAutoCompleteWhenLoadingCompletes = false;
		auto loadingWidget = CreateWidget<UUserWidget>(this, WidgetLoadingScreen);
		
		if(SoftAssertTrue(loadingWidget != nullptr, GetWorld(), __FILE__, __LINE__,"No Loading Screen Widget"))
			LoadingScreen.WidgetLoadingScreen = loadingWidget->TakeWidget();

		GetMoviePlayer()->SetupLoadingScreen(LoadingScreen);
	}
}

void UInflectionPointGameInstanceBase::EndLoadingScreen(UWorld* InLoadedWorld) {DebugPrint(__FILE__, __LINE__);
	UE_LOG(LogTemp, Warning, TEXT("EndLoadingScreen"));
}