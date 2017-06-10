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
		JoinSession(Player->GetPreferredUniqueNetId(), GameSessionName/*SessionSearch->SearchResults[i].Session.OwningUserName)*/, OnlineSessionSearchResult);
	}
}

bool USessionSearchResultBase::JoinSession(TSharedPtr<const FUniqueNetId> UserId, FName SessionName, const FOnlineSessionSearchResult& SearchResult) {
	bool bSuccessful = false;
	IOnlineSessionPtr Sessions = GetSessionInterface();

	if(Sessions.IsValid() && UserId.IsValid()) {
		OnJoinSessionCompleteDelegateHandle = Sessions->AddOnJoinSessionCompleteDelegate_Handle(OnJoinSessionCompleteDelegate);
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