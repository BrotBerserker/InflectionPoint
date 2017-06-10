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
	USessionSearchResultBase(const FObjectInitializer& ObjectInitializer);

	FOnlineSessionSearchResult OnlineSessionSearchResult;

	UFUNCTION(BlueprintCallable, Category = "InflectionPoint|Networking")
		void JoinOnlineGame();

	UFUNCTION(BlueprintCallable, Category = "InflectionPoint|Networking")
		FString GetOnlineSessionName();

	UFUNCTION(BlueprintImplementableEvent, Category = "InflectionPoint|Networking")
		void OnJoinComplete();

private:
	FOnJoinSessionCompleteDelegate OnJoinSessionCompleteDelegate;
	FDelegateHandle OnJoinSessionCompleteDelegateHandle;

	bool JoinSession(TSharedPtr<const FUniqueNetId> UserId, FName SessionName, const FOnlineSessionSearchResult& SearchResult);

	void OnJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result);

	IOnlineSessionPtr GetSessionInterface();
};
