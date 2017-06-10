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
	UJoinMenuBase();

	UFUNCTION(BlueprintCallable, Category = "InflectionPoint|Networking")
		void FindOnlineGames(bool isLan);

	UFUNCTION(BlueprintCallable, Category = "InflectionPoint|Networking")
		void JoinOnlineGame(int index = 0);

	UFUNCTION(BlueprintImplementableEvent, Category = "InflectionPoint|Networking")
		void OnSessionFound();

	UFUNCTION(BlueprintImplementableEvent, Category = "InflectionPoint|Networking")
		void OnSessionSearchComplete();

	UFUNCTION(BlueprintImplementableEvent, Category = "InflectionPoint|Networking")
		void OnJoinComplete();

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "InflectionPoint|Networking")
		TArray<USessionSearchResultBase*> SessionSearchResults;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
		TSubclassOf<USessionSearchResultBase> SessionSearchResultType;

	//UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "InflectionPoint|Networking")
	//	SearchResultsWrapper wrapper;

private:
	TSharedPtr<class FOnlineSessionSearch> SessionSearch;
	FOnFindSessionsCompleteDelegate OnFindSessionsCompleteDelegate;
	FDelegateHandle OnFindSessionsCompleteDelegateHandle;

	FOnJoinSessionCompleteDelegate OnJoinSessionCompleteDelegate;
	FDelegateHandle OnJoinSessionCompleteDelegateHandle;

	void FindSessions(TSharedPtr<const FUniqueNetId> UserId, FName SessionName, bool bIsLAN, bool bIsPresence);

	void OnFindSessionsComplete(bool bWasSuccessful);

	bool JoinSession(TSharedPtr<const FUniqueNetId> UserId, FName SessionName, const FOnlineSessionSearchResult& SearchResult);

	void OnJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result);

	IOnlineSessionPtr GetSessionInterface();
};
