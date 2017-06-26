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

	UFUNCTION(BlueprintImplementableEvent, Category = "InflectionPoint|Networking")
		void OnSessionFound();

	UFUNCTION(BlueprintImplementableEvent, Category = "InflectionPoint|Networking")
		void OnSessionSearchComplete();

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "InflectionPoint|Networking")
		TArray<USessionSearchResultBase*> SessionSearchResults;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
		TSubclassOf<USessionSearchResultBase> SessionSearchResultType;

private:
	TSharedPtr<class FOnlineSessionSearch> SessionSearch;
	FOnFindSessionsCompleteDelegate OnFindSessionsCompleteDelegate;
	FDelegateHandle OnFindSessionsCompleteDelegateHandle;

	void FindSessions(TSharedPtr<const FUniqueNetId> UserId, FName SessionName, bool bIsLAN, bool bIsPresence);

	void OnFindSessionsComplete(bool bWasSuccessful);

	TArray<USessionSearchResultBase*> CreateSessionSearchResultWidgets(TSharedPtr<const FUniqueNetId> currentUniqueNetId);

	IOnlineSessionPtr GetSessionInterface();
};
