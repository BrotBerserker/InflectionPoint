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
	/* ------------- */
	/*   Functions   */
	/* ------------- */

	/** Constructor, sets up delegates */
	UHostMenuBase();

	/** Start a new server for a multiplayer match */
	UFUNCTION(BlueprintCallable, Category = "InflectionPoint|Networking")
		void HostServer(int playerAmount, bool lan, FString serverName, FString levelToOpen, int rounds);

	/** Create a new session */
	bool HostSession(TSharedPtr<const FUniqueNetId> UserId, FName SessionName, bool bIsLAN, bool bIsPresence, int32 MaxNumPlayers, int32 Rounds);

	/** Sets up a bunch of settings for the next session to be created */
	void SetupSessionSettings(bool bIsLAN, bool bIsPresence, const int32 &MaxNumPlayers, FName &SessionName, int Rounds);

	/** Gets the SessionInterface using the current OnlineSubsystem (Steam) */
	IOnlineSessionPtr GetSessionInterface();

public:
	/* -------------- */
	/*   Properties   */
	/* -------------- */

	/** Name of the level that will be opened after creating a session */
	FString LevelToOpen;

	TSharedPtr<class FOnlineSessionSettings> SessionSettings;

private:
	/** Delegate called when session created */
	FOnCreateSessionCompleteDelegate OnCreateSessionCompleteDelegate;

	/** Delegate called when session started */
	FOnStartSessionCompleteDelegate OnStartSessionCompleteDelegate;

	/** Handles to registered delegates for creating/starting a session */
	FDelegateHandle OnCreateSessionCompleteDelegateHandle;
	FDelegateHandle OnStartSessionCompleteDelegateHandle;

	/** Functions registered as delegates */
	virtual void OnCreateSessionComplete(FName SessionName, bool bWasSuccessful);
	void OnStartOnlineGameComplete(FName SessionName, bool bWasSuccessful);
};
