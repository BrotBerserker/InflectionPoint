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
	int32 maxPlayers = OnlineSessionSearchResult.Session.SessionSettings.NumPublicConnections;
	int32 connectedPlayers = maxPlayers - OnlineSessionSearchResult.Session.NumOpenPrivateConnections;
	return sessionName;
}

int32 USessionSearchResultBase::GetMaxPlayers() {
	return OnlineSessionSearchResult.Session.SessionSettings.NumPublicConnections;
}

int32 USessionSearchResultBase::GetConnectedPlayers() {
	int32 currentPlayers;
	OnlineSessionSearchResult.Session.SessionSettings.Get(FName("CurrentPlayers"), currentPlayers);
	return currentPlayers;
}

FString USessionSearchResultBase::GetMapName() {
	FString mapName;
	OnlineSessionSearchResult.Session.SessionSettings.Get(SETTING_MAPNAME, mapName);
	return mapName;
}

void USessionSearchResultBase::JoinOnlineGame() {
	ULocalPlayer* const Player = GetWorld()->GetFirstLocalPlayerFromController();

	if(OnlineSessionSearchResult.Session.OwningUserId != Player->GetPreferredUniqueNetId()) {
		JoinSession(Player->GetPreferredUniqueNetId(), GameSessionName, OnlineSessionSearchResult);
	}
}

bool USessionSearchResultBase::JoinSession(TSharedPtr<const FUniqueNetId> UserId, FName SessionName, const FOnlineSessionSearchResult& SearchResult) {
	IOnlineSessionPtr Sessions = GetSessionInterface();

	if(Sessions.IsValid() && UserId.IsValid()) {
		OnJoinSessionCompleteDelegateHandle = Sessions->AddOnJoinSessionCompleteDelegate_Handle(OnJoinSessionCompleteDelegate);
		Sessions->JoinSession(*UserId, SessionName, SearchResult);
		return true;
	}
	return false;
}

void USessionSearchResultBase::OnJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result) {
	IOnlineSessionPtr Sessions = GetSessionInterface();
	if(Sessions.IsValid()) {
		Sessions->ClearOnJoinSessionCompleteDelegate_Handle(OnJoinSessionCompleteDelegateHandle);

		APlayerController * const PlayerController = GetWorld()->GetFirstPlayerController();
		FString TravelURL;

		if(PlayerController && Sessions->GetResolvedConnectString(SessionName, TravelURL)) {
			PlayerController->ClientTravel(TravelURL, ETravelType::TRAVEL_Absolute);
		}
		OnJoinComplete(SessionName);
	}
}

IOnlineSessionPtr USessionSearchResultBase::GetSessionInterface() {
	IOnlineSubsystem* const OnlineSub = IOnlineSubsystem::Get();

	if(!AssertNotNull(OnlineSub, GetWorld(), __FILE__, __LINE__))
		return IOnlineSessionPtr();
	return OnlineSub->GetSessionInterface();
}