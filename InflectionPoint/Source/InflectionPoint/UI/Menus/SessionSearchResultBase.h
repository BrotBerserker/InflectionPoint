// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Blueprint/UserWidget.h"
#include "OnlineSubsystemUtils.h"
#include "SessionSearchResultBase.generated.h"

/**
 *
 */
UCLASS()
class INFLECTIONPOINT_API USessionSearchResultBase : public UUserWidget {
	GENERATED_BODY()

public:
	/* ------------- */
	/*   Functions   */
	/* ------------- */

	USessionSearchResultBase(const FObjectInitializer& ObjectInitializer);

	UFUNCTION(BlueprintCallable, Category = "InflectionPoint|Networking")
		void JoinOnlineGame();

	UFUNCTION(BlueprintCallable, Category = "InflectionPoint|Networking")
		FString GetOnlineSessionName();

	UFUNCTION(BlueprintCallable, Category = "InflectionPoint|Networking")
		int32 GetMaxPlayers();

	UFUNCTION(BlueprintCallable, Category = "InflectionPoint|Networking")
		int32 GetConnectedPlayers();

	UFUNCTION(BlueprintImplementableEvent, Category = "InflectionPoint|Networking")
		void OnJoinComplete(FName SessioName);

public:
	/* -------------- */
	/*   Properties   */
	/* -------------- */

	FOnlineSessionSearchResult OnlineSessionSearchResult;

private:
	bool JoinSession(TSharedPtr<const FUniqueNetId> UserId, FName SessionName, const FOnlineSessionSearchResult& SearchResult);
	IOnlineSessionPtr GetSessionInterface();

private:
	/** Handle and Delegate to join a session */
	FOnJoinSessionCompleteDelegate OnJoinSessionCompleteDelegate;
	FDelegateHandle OnJoinSessionCompleteDelegateHandle;

	/** Function registered as delegates */
	void OnJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result);

};
