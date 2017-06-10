// Fill out your copyright notice in the Description page of Project Settings.

#include "InflectionPoint.h"
#include "SessionSearchResultBase.h"


USessionSearchResultBase::USessionSearchResultBase(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer) {
	OnJoinSessionCompleteDelegate = FOnJoinSessionCompleteDelegate::CreateUObject(this, &USessionSearchResultBase::OnJoinSessionComplete);
}

FString USessionSearchResultBase::GetOnlineSessionName() {
	FString sessionName;
	OnlineSessionSearchResult.Session.SessionSettings.Get(FName("SessionName"), sessionName);
	return sessionName;
}

void USessionSearchResultBase::JoinOnlineGame() {
	ULocalPlayer* const Player = GetWorld()->GetFirstLocalPlayerFromController();

	if(OnlineSessionSearchResult.Session.OwningUserId != Player->GetPreferredUniqueNetId()) {
		// Once we found sounce a Session that is not ours, just join it. Instead of using a for loop, you could
		// use a widget where you click on and have a reference for the GameSession it represents which you can use
		// here
		JoinSession(Player->GetPreferredUniqueNetId(), GameSessionName/*SessionSearch->SearchResults[i].Session.OwningUserName)*/, OnlineSessionSearchResult);
	}
}

bool USessionSearchResultBase::JoinSession(TSharedPtr<const FUniqueNetId> UserId, FName SessionName, const FOnlineSessionSearchResult& SearchResult) {
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

void USessionSearchResultBase::OnJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result) {
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

IOnlineSessionPtr USessionSearchResultBase::GetSessionInterface() {
	IOnlineSubsystem* const OnlineSub = IOnlineSubsystem::Get();

	if(!AssertNotNull(OnlineSub, GetWorld(), __FILE__, __LINE__))
		return IOnlineSessionPtr();
	return OnlineSub->GetSessionInterface();
}