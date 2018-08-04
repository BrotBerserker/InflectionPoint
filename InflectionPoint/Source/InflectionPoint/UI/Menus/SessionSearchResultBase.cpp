// Fill out your copyright notice in the Description page of Project Settings.

#include "InflectionPoint.h"
#include "SessionSearchResultBase.h"


USessionSearchResultBase::USessionSearchResultBase(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer) {DebugPrint(__FILE__, __LINE__);
	OnJoinSessionCompleteDelegate = FOnJoinSessionCompleteDelegate::CreateUObject(this, &USessionSearchResultBase::OnJoinSessionComplete);
}

FString USessionSearchResultBase::GetOnlineSessionName() {DebugPrint(__FILE__, __LINE__);
	FString sessionName;
	OnlineSessionSearchResult.Session.SessionSettings.Get(FName("SessionName"), sessionName);
	int32 maxPlayers = OnlineSessionSearchResult.Session.SessionSettings.NumPublicConnections;
	int32 connectedPlayers = maxPlayers - OnlineSessionSearchResult.Session.NumOpenPrivateConnections;
	return sessionName;
}

int32 USessionSearchResultBase::GetMaxPlayers() {DebugPrint(__FILE__, __LINE__);
	return OnlineSessionSearchResult.Session.SessionSettings.NumPublicConnections;
}

int32 USessionSearchResultBase::GetConnectedPlayers() {DebugPrint(__FILE__, __LINE__);
	int32 currentPlayers;
	OnlineSessionSearchResult.Session.SessionSettings.Get(FName("CurrentPlayers"), currentPlayers);
	return currentPlayers;
}

FString USessionSearchResultBase::GetMapName() {DebugPrint(__FILE__, __LINE__);
	FString mapName;
	OnlineSessionSearchResult.Session.SessionSettings.Get(SETTING_MAPNAME, mapName);
	return mapName;
}

void USessionSearchResultBase::JoinOnlineGame() {DebugPrint(__FILE__, __LINE__);
	ULocalPlayer* const Player = GetWorld()->GetFirstLocalPlayerFromController();

	if(OnlineSessionSearchResult.Session.OwningUserId != Player->GetPreferredUniqueNetId()) {DebugPrint(__FILE__, __LINE__);
		JoinSession(Player->GetPreferredUniqueNetId(), GameSessionName, OnlineSessionSearchResult);
	}
}

bool USessionSearchResultBase::JoinSession(TSharedPtr<const FUniqueNetId> UserId, FName SessionName, const FOnlineSessionSearchResult& SearchResult) {DebugPrint(__FILE__, __LINE__);
	IOnlineSessionPtr Sessions = GetSessionInterface();

	if(Sessions.IsValid() && UserId.IsValid()) {DebugPrint(__FILE__, __LINE__);
		OnJoinSessionCompleteDelegateHandle = Sessions->AddOnJoinSessionCompleteDelegate_Handle(OnJoinSessionCompleteDelegate);
		Sessions->JoinSession(*UserId, SessionName, SearchResult);
		return true;
	}
	return false;
}

void USessionSearchResultBase::OnJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result) {DebugPrint(__FILE__, __LINE__);
	IOnlineSessionPtr Sessions = GetSessionInterface();
	if(Sessions.IsValid()) {DebugPrint(__FILE__, __LINE__);
		Sessions->ClearOnJoinSessionCompleteDelegate_Handle(OnJoinSessionCompleteDelegateHandle);

		APlayerController * const PlayerController = GetWorld()->GetFirstPlayerController();
		FString TravelURL;

		if(PlayerController && Sessions->GetResolvedConnectString(SessionName, TravelURL)) {DebugPrint(__FILE__, __LINE__);
			PlayerController->ClientTravel(TravelURL, ETravelType::TRAVEL_Absolute);
		}
		OnJoinComplete(SessionName);
	}
}

IOnlineSessionPtr USessionSearchResultBase::GetSessionInterface() {DebugPrint(__FILE__, __LINE__);
	IOnlineSubsystem* const OnlineSub = IOnlineSubsystem::Get();

	if(!AssertNotNull(OnlineSub, GetWorld(), __FILE__, __LINE__))
		return IOnlineSessionPtr();
	return OnlineSub->GetSessionInterface();
}