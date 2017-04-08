// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "SubMenuTemplate.h"
#include "OnlineSubsystemUtils.h"
#include "Blueprint/UserWidget.h"
#include "JoinMenuBase.generated.h"


/**
 *
 */
UCLASS()
class INFLECTIONPOINT_API UJoinMenuBase : public USubMenuTemplate {
	GENERATED_BODY()

public:
	UJoinMenuBase();

	UFUNCTION(BlueprintCallable, Category = "Network")
		void FindOnlineGames(bool isLan);

	UFUNCTION(BlueprintCallable, Category = "Network")
		void JoinOnlineGame();

	UFUNCTION(BlueprintImplementableEvent, Category = "Network")
		void OnSessionFound();

	UFUNCTION(BlueprintImplementableEvent, Category = "Network")
		void OnSessionSearchComplete();

	UFUNCTION(BlueprintImplementableEvent, Category = "Network")
		void OnJoinComplete();



	void FindSessions(TSharedPtr<const FUniqueNetId> UserId, FName SessionName, bool bIsLAN, bool bIsPresence);

	/** Delegate for searching for sessions */
	FOnFindSessionsCompleteDelegate OnFindSessionsCompleteDelegate;

	/** Handle to registered delegate for searching a session */
	FDelegateHandle OnFindSessionsCompleteDelegateHandle;

	TSharedPtr<class FOnlineSessionSearch> SessionSearch;

	void OnFindSessionsComplete(bool bWasSuccessful);

	bool JoinSession(TSharedPtr<const FUniqueNetId> UserId, FName SessionName, const FOnlineSessionSearchResult& SearchResult);

	/** Delegate for joining a session */
	FOnJoinSessionCompleteDelegate OnJoinSessionCompleteDelegate;

	/** Handle to registered delegate for joining a session */
	FDelegateHandle OnJoinSessionCompleteDelegateHandle;

	void OnJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result);
};
