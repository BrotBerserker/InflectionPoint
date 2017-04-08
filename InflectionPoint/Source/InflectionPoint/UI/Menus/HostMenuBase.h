// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "SubMenuTemplate.h"
#include "OnlineSubsystemUtils.h"
#include "Blueprint/UserWidget.h"
#include "HostMenuBase.generated.h"

/**
 *
 */
UCLASS()
class INFLECTIONPOINT_API UHostMenuBase : public USubMenuTemplate {
	GENERATED_BODY()

public:

	UHostMenuBase();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Game")
		FString NameOfLevelToOpen;


	UFUNCTION(BlueprintCallable, Category = "Game")
		void HostServer(int playerAmount, bool lan, FString serverName);

	bool HostSession(TSharedPtr<const FUniqueNetId> UserId, FName SessionName, bool bIsLAN, bool bIsPresence, int32 MaxNumPlayers);
	/* Delegate called when session created */
	FOnCreateSessionCompleteDelegate OnCreateSessionCompleteDelegate;
	/* Delegate called when session started */
	FOnStartSessionCompleteDelegate OnStartSessionCompleteDelegate;

	/** Handles to registered delegates for creating/starting a session */
	FDelegateHandle OnCreateSessionCompleteDelegateHandle;
	FDelegateHandle OnStartSessionCompleteDelegateHandle;

	TSharedPtr<class FOnlineSessionSettings> SessionSettings;

	virtual void OnCreateSessionComplete(FName SessionName, bool bWasSuccessful);

	void OnStartOnlineGameComplete(FName SessionName, bool bWasSuccessful);
};
