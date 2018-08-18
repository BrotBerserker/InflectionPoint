// Fill out your copyright notice in the Description page of Project Settings.
// Tutorial link: https://wiki.unrealengine.com/How_To_Use_Sessions_In_C%2B%2B

#include "InflectionPoint.h"
#include "Runtime/UMG/Public/UMG.h"
#include "Runtime/UMG/Public/Blueprint/WidgetBlueprintLibrary.h"
#include "JoinMenuBase.h"


UJoinMenuBase::UJoinMenuBase() : Super() {
	OnFindSessionsCompleteDelegate = FOnFindSessionsCompleteDelegate::CreateUObject(this, &UJoinMenuBase::OnFindSessionsComplete);
	static ConstructorHelpers::FObjectFinder<UClass> SessionSearchResult(TEXT("Class'/Game/InflectionPoint/UI/Menus/Widgets/SessionSearchResult.SessionSearchResult_C'"));
	if(SessionSearchResult.Object != NULL) {
		SessionSearchResultType = SessionSearchResult.Object;
	}
}

void UJoinMenuBase::FindOnlineGames(bool isLan) {
	ULocalPlayer* const Player = GetWorld()->GetFirstLocalPlayerFromController();
	FindSessions(Player->GetPreferredUniqueNetId(), GameSessionName, isLan, true);
}

void UJoinMenuBase::FindSessions(FUniqueNetIdRepl UserId, FName SessionName, bool bIsLAN, bool bIsPresence) {
	IOnlineSessionPtr Sessions = GetSessionInterface();
	if(Sessions.IsValid() && UserId.IsValid()) {
		SessionSearch = MakeShareable(new FOnlineSessionSearch());
		SessionSearch->bIsLanQuery = bIsLAN;
		SessionSearch->MaxSearchResults = 9001;
		SessionSearch->PingBucketSize = 50;

		if(bIsPresence) {
			SessionSearch->QuerySettings.Set(SEARCH_PRESENCE, bIsPresence, EOnlineComparisonOp::Equals);
		}

		TSharedRef<FOnlineSessionSearch> SearchSettingsRef = SessionSearch.ToSharedRef();
		OnFindSessionsCompleteDelegateHandle = Sessions->AddOnFindSessionsCompleteDelegate_Handle(OnFindSessionsCompleteDelegate);
		Sessions->FindSessions(*UserId, SearchSettingsRef);
	}
}

void UJoinMenuBase::OnFindSessionsComplete(bool bWasSuccessful) {
	OnSessionSearchComplete();

	ULocalPlayer* const Player = GetWorld()->GetFirstLocalPlayerFromController();
	IOnlineSessionPtr Sessions = GetSessionInterface();

	if(Sessions.IsValid()) {
		// Clear the Delegate handle
		Sessions->ClearOnFindSessionsCompleteDelegate_Handle(OnFindSessionsCompleteDelegateHandle);
		// Create Widgets for the Server-List
		CreateSessionSearchResultWidgets(Player->GetPreferredUniqueNetId());

		if(SessionSearchResultWidgets.Num() > 0) {
			OnSessionFound();
		}
	}
}

void UJoinMenuBase::CreateSessionSearchResultWidgets(FUniqueNetIdRepl currentUniqueNetId) {
	SessionSearchResultWidgets = TArray<USessionSearchResultBase*>();
	for(auto &searchResult : SessionSearch->SearchResults) {
		if(searchResult.Session.OwningUserId == currentUniqueNetId)
			continue;

		SessionSearchResultWidgets.Add(CreateSessionSearchResultWidget(searchResult));
	}
}

USessionSearchResultBase * UJoinMenuBase::CreateSessionSearchResultWidget(FOnlineSessionSearchResult searchResult) {
	FString sessionName;
	searchResult.Session.SessionSettings.Get(FName("SessionName"), sessionName);

	// Create the Widget for the Server-List and set the session search result
	USessionSearchResultBase* SessionSearchResultWidget = (USessionSearchResultBase*)CreateWidget<USessionSearchResultBase>(GetWorld(), SessionSearchResultType);
	SessionSearchResultWidget->OnlineSessionSearchResult = searchResult;
	return SessionSearchResultWidget;
}

IOnlineSessionPtr UJoinMenuBase::GetSessionInterface() {
	IOnlineSubsystem* const OnlineSub = IOnlineSubsystem::Get();

	if(!AssertNotNull(OnlineSub, GetWorld(), __FILE__, __LINE__))
		return IOnlineSessionPtr();
	return OnlineSub->GetSessionInterface();
}

