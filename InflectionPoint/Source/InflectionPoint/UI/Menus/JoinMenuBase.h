// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "SubMenuTemplate.h"
#include "OnlineSubsystemUtils.h"
#include "Blueprint/UserWidget.h"
#include "SessionSearchResultBase.h"
#include "JoinMenuBase.generated.h"


/**
 *
 */
UCLASS()
class INFLECTIONPOINT_API UJoinMenuBase : public USubMenuTemplate {
	GENERATED_BODY()

public:
	/* ---------------------- */
	/*  Blueprint Properties  */
	/* ---------------------- */

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "InflectionPoint|Networking")
		TArray<USessionSearchResultBase*> SessionSearchResultWidgets;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
		TSubclassOf<USessionSearchResultBase> SessionSearchResultType;

public:
	/* ------------- */
	/*   Functions   */
	/* ------------- */

	UJoinMenuBase();

	UFUNCTION(BlueprintCallable, Category = "InflectionPoint|Networking")
		void FindOnlineGames(bool isLan);

	UFUNCTION(BlueprintImplementableEvent, Category = "InflectionPoint|Networking")
		void OnSessionFound();

	UFUNCTION(BlueprintImplementableEvent, Category = "InflectionPoint|Networking")
		void OnSessionSearchComplete();

private:
	TSharedPtr<class FOnlineSessionSearch> SessionSearch;

	void FindSessions(FUniqueNetIdRepl UserId, FName SessionName, bool bIsLAN, bool bIsPresence);
	
	void CreateSessionSearchResultWidgets(FUniqueNetIdRepl currentUniqueNetId);

	USessionSearchResultBase* CreateSessionSearchResultWidget(FOnlineSessionSearchResult session);

	IOnlineSessionPtr GetSessionInterface();

private:
	/** Handle and Delegate to search for sessions */
	FOnFindSessionsCompleteDelegate OnFindSessionsCompleteDelegate;
	FDelegateHandle OnFindSessionsCompleteDelegateHandle;

	/** Function registered as delegates */
	void OnFindSessionsComplete(bool bWasSuccessful);
};
