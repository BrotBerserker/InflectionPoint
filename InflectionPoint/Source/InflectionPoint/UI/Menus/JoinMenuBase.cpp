// Fill out your copyright notice in the Description page of Project Settings.
// Tutorial link: https://wiki.unrealengine.com/How_To_Use_Sessions_In_C%2B%2B

#include "InflectionPoint.h"
#include "JoinMenuBase.h"


UJoinMenuBase::UJoinMenuBase() : Super() {
	OnFindSessionsCompleteDelegate = FOnFindSessionsCompleteDelegate::CreateUObject(this, &UJoinMenuBase::OnFindSessionsComplete);
	OnJoinSessionCompleteDelegate = FOnJoinSessionCompleteDelegate::CreateUObject(this, &UJoinMenuBase::OnJoinSessionComplete);
}

void UJoinMenuBase::FindOnlineGames(bool isLan) {
	ULocalPlayer* const Player = GetWorld()->GetFirstLocalPlayerFromController();
	FindSessions(Player->GetPreferredUniqueNetId(), GameSessionName, isLan, true);
}


void UJoinMenuBase::JoinOnlineGame() {
	ULocalPlayer* const Player = GetWorld()->GetFirstLocalPlayerFromController();
	FOnlineSessionSearchResult SearchResult;

	for(int32 i = 0; i < SessionSearch->SearchResults.Num(); i++) {
		if(SessionSearch->SearchResults[i].Session.OwningUserId != Player->GetPreferredUniqueNetId()) {
			SearchResult = SessionSearch->SearchResults[i];

			// Once we found sounce a Session that is not ours, just join it. Instead of using a for loop, you could
			// use a widget where you click on and have a reference for the GameSession it represents which you can use
			// here
			JoinSession(Player->GetPreferredUniqueNetId(), GameSessionName/*SessionSearch->SearchResults[i].Session.OwningUserName)*/, SearchResult);
			break;
		}		
	}
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
	GEngine->AddOnScreenDebugMessage(-1, 100.f, FColor::Red, FString::Printf(TEXT("OFindSessionsComplete bSuccess: %d"), bWasSuccessful));
	OnSessionSearchComplete();

	ULocalPlayer* const Player = GetWorld()->GetFirstLocalPlayerFromController();	
	IOnlineSessionPtr Sessions = GetSessionInterface();

	if(Sessions.IsValid()) {
		// Clear the Delegate handle
		Sessions->ClearOnFindSessionsCompleteDelegate_Handle(OnFindSessionsCompleteDelegateHandle);

		GEngine->AddOnScreenDebugMessage(-1, 100.f, FColor::Red, FString::Printf(TEXT("Num Search Results: %d (%d without self)"), SessionSearch->SearchResults.Num(), SessionSearch->SearchResults.Num()-1));

		// If we have found at least 1 session, we just going to debug them. You could add them to a list of UMG Widgets, like it is done in the BP version!			
		bool sessionFound = false;
		for(int32 SearchIdx = 0; SearchIdx < SessionSearch->SearchResults.Num(); SearchIdx++) {
			if(SessionSearch->SearchResults[SearchIdx].Session.OwningUserId != Player->GetPreferredUniqueNetId()) {
				sessionFound = true;
				FString sessionName;
				SessionSearch->SearchResults[SearchIdx].Session.SessionSettings.Get(FName("SessionName"), sessionName);
				GEngine->AddOnScreenDebugMessage(-1, 100.f, FColor::Red, FString::Printf(TEXT("Session Number: %d | Sessionname: %s "), SearchIdx + 1, *sessionName));
			} else {
				FString sessionName;
				SessionSearch->SearchResults[SearchIdx].Session.SessionSettings.Get(FName("SessionName"), sessionName);
				GEngine->AddOnScreenDebugMessage(-1, 100.f, FColor::Red, FString::Printf(TEXT("Self: Session Number: %d | Sessionname: %s "), SearchIdx + 1, *sessionName));
			}
		}
		if(sessionFound)
			OnSessionFound();		
	}
}

IOnlineSessionPtr UJoinMenuBase::GetSessionInterface() {
	IOnlineSubsystem* const OnlineSub = IOnlineSubsystem::Get();

	if(!AssertNotNull(OnlineSub, GetWorld(), __FILE__, __LINE__))
		return IOnlineSessionPtr();
	return OnlineSub->GetSessionInterface();
}


bool UJoinMenuBase::JoinSession(TSharedPtr<const FUniqueNetId> UserId, FName SessionName, const FOnlineSessionSearchResult& SearchResult) {
	bool bSuccessful = false;
	IOnlineSessionPtr Sessions = GetSessionInterface();

	if(Sessions.IsValid() && UserId.IsValid()) {
		OnJoinSessionCompleteDelegateHandle = Sessions->AddOnJoinSessionCompleteDelegate_Handle(OnJoinSessionCompleteDelegate);

		// Call the "JoinSession" Function with the passed "SearchResult". The "SessionSearch->SearchResults" can be used to get such a
		// "FOnlineSessionSearchResult" and pass it.
		bSuccessful = Sessions->JoinSession(*UserId, SessionName, SearchResult);
	}
	return bSuccessful;
}

void UJoinMenuBase::OnJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result) {
	GEngine->AddOnScreenDebugMessage(-1, 100.f, FColor::Red, FString::Printf(TEXT("OnJoinSessionComplete %s, %d"), *SessionName.ToString(), static_cast<int32>(Result)));
	
	IOnlineSessionPtr Sessions = GetSessionInterface();
	if(Sessions.IsValid()) {
		Sessions->ClearOnJoinSessionCompleteDelegate_Handle(OnJoinSessionCompleteDelegateHandle);

		APlayerController * const PlayerController = GetWorld()->GetFirstPlayerController();

		// We need a FString to use ClientTravel and we can let the SessionInterface contruct such a
		// String for us by giving him the SessionName and an empty String. We want to do this, because
		// Every OnlineSubsystem uses different TravelURLs
		FString TravelURL;

		if(PlayerController && Sessions->GetResolvedConnectString(SessionName, TravelURL)) {
			PlayerController->ClientTravel(TravelURL, ETravelType::TRAVEL_Absolute);
		}
		OnJoinComplete();		
	}
}
