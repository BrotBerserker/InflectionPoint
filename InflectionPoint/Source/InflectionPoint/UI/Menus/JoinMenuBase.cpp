// Fill out your copyright notice in the Description page of Project Settings.
// Tutorial link: https://wiki.unrealengine.com/How_To_Use_Sessions_In_C%2B%2B

#include "InflectionPoint.h"
#include "Runtime/UMG/Public/UMG.h"
#include "Runtime/UMG/Public/Blueprint/WidgetBlueprintLibrary.h"
#include "JoinMenuBase.h"


UJoinMenuBase::UJoinMenuBase() : Super() {
	OnFindSessionsCompleteDelegate = FOnFindSessionsCompleteDelegate::CreateUObject(this, &UJoinMenuBase::OnFindSessionsComplete);
	static ConstructorHelpers::FObjectFinder<UClass> SessionSearchResult(TEXT("Class'/Game/InflectionPoint/UI/Menus/SessionSearchResult.SessionSearchResult_C'"));
	if(SessionSearchResult.Object != NULL) {
		SessionSearchResultType = SessionSearchResult.Object;
	}
}

void UJoinMenuBase::FindOnlineGames(bool isLan) {
	ULocalPlayer* const Player = GetWorld()->GetFirstLocalPlayerFromController();
	FindSessions(Player->GetPreferredUniqueNetId(), GameSessionName, isLan, true);
}

void UJoinMenuBase::FindSessions(TSharedPtr<const FUniqueNetId> UserId, FName SessionName, bool bIsLAN, bool bIsPresence) {
	IOnlineSessionPtr Sessions = GetSessionInterface();
	if(Sessions.IsValid() && UserId.IsValid()) {
		SessionSearch = MakeShareable(new FOnlineSessionSearch());
		SessionSearch->bIsLanQuery = bIsLAN;
		SessionSearch->MaxSearchResults = 20;
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
	//GEngine->AddOnScreenDebugMessage(-1, 100.f, FColor::Red, FString::Printf(TEXT("OFindSessionsComplete bSuccess: %d"), bWasSuccessful));
	OnSessionSearchComplete();

	ULocalPlayer* const Player = GetWorld()->GetFirstLocalPlayerFromController();
	IOnlineSessionPtr Sessions = GetSessionInterface();

	if(Sessions.IsValid()) {
		// Clear the Delegate handle
		Sessions->ClearOnFindSessionsCompleteDelegate_Handle(OnFindSessionsCompleteDelegateHandle);

		//GEngine->AddOnScreenDebugMessage(-1, 100.f, FColor::Red, FString::Printf(TEXT("Num Search Results: %d (%d without self)"), SessionSearch->SearchResults.Num(), SessionSearch->SearchResults.Num() - 1));

		auto results = TArray<USessionSearchResultBase*>();
		// If we have found at least 1 session, we just going to debug them. You could add them to a list of UMG Widgets, like it is done in the BP version!			
		bool sessionFound = false;
		for(int32 SearchIdx = 0; SearchIdx < SessionSearch->SearchResults.Num(); SearchIdx++) {
			if(SessionSearch->SearchResults[SearchIdx].Session.OwningUserId != Player->GetPreferredUniqueNetId()) {
				sessionFound = true;
				FString sessionName;
				SessionSearch->SearchResults[SearchIdx].Session.SessionSettings.Get(FName("SessionName"), sessionName);
				USessionSearchResultBase* SessionSearchResult = (USessionSearchResultBase*)CreateWidget<USessionSearchResultBase>(GetWorld(), SessionSearchResultType);
				SessionSearchResult->OnlineSessionSearchResult = SessionSearch->SearchResults[SearchIdx];
				results.Add(SessionSearchResult);
				//GEngine->AddOnScreenDebugMessage(-1, 100.f, FColor::Red, FString::Printf(TEXT("Session Number: %d | Sessionname: %s "), SearchIdx + 1, *sessionName));
			} else {
				FString sessionName;
				SessionSearch->SearchResults[SearchIdx].Session.SessionSettings.Get(FName("SessionName"), sessionName);
				//GEngine->AddOnScreenDebugMessage(-1, 100.f, FColor::Red, FString::Printf(TEXT("Self: Session Number: %d | Sessionname: %s "), SearchIdx + 1, *sessionName));
			}
		}
		if(sessionFound) {
			SessionSearchResults = results;
			OnSessionFound();
			//GEngine->AddOnScreenDebugMessage(-1, 100.f, FColor::Red, FString::Printf(TEXT("OnSessionFound called")));
		}
	}
}

IOnlineSessionPtr UJoinMenuBase::GetSessionInterface() {
	IOnlineSubsystem* const OnlineSub = IOnlineSubsystem::Get();

	if(!AssertNotNull(OnlineSub, GetWorld(), __FILE__, __LINE__))
		return IOnlineSessionPtr();
	return OnlineSub->GetSessionInterface();
}

