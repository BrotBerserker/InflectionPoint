// Fill out your copyright notice in the Description page of Project Settings.

#include "InflectionPoint.h"
#include "HostMenuBase.h"

UHostMenuBase::UHostMenuBase() : Super() {DebugPrint(__FILE__, __LINE__);
	OnCreateSessionCompleteDelegate = FOnCreateSessionCompleteDelegate::CreateUObject(this, &UHostMenuBase::OnCreateSessionComplete);
	OnStartSessionCompleteDelegate = FOnStartSessionCompleteDelegate::CreateUObject(this, &UHostMenuBase::OnStartOnlineGameComplete);
}

void UHostMenuBase::HostServer(int playerAmount, bool isLan, FString serverName, FString levelToOpen, int rounds) {DebugPrint(__FILE__, __LINE__);
	this->LevelToOpen = levelToOpen;

	ULocalPlayer* const Player = GetWorld()->GetFirstLocalPlayerFromController();

	HostSession(Player->GetPreferredUniqueNetId(), FName(*serverName), isLan, true, playerAmount, rounds);
}

IOnlineSessionPtr UHostMenuBase::GetSessionInterface() {DebugPrint(__FILE__, __LINE__);
	IOnlineSubsystem* const OnlineSub = IOnlineSubsystem::Get();

	AssertNotNull(OnlineSub, GetWorld(), __FILE__, __LINE__);

	return OnlineSub->GetSessionInterface();
}

bool UHostMenuBase::HostSession(TSharedPtr<const FUniqueNetId> UserId, FName SessionName, bool bIsLAN, bool bIsPresence, int32 MaxNumPlayers, int32 Rounds) {DebugPrint(__FILE__, __LINE__);
	IOnlineSessionPtr Sessions = GetSessionInterface();

	if(Sessions.IsValid() && UserId.IsValid()) {DebugPrint(__FILE__, __LINE__);
		SetupSessionSettings(bIsLAN, bIsPresence, MaxNumPlayers, SessionName, Rounds);

		OnCreateSessionCompleteDelegateHandle = Sessions->AddOnCreateSessionCompleteDelegate_Handle(OnCreateSessionCompleteDelegate);

		return Sessions->CreateSession(*UserId, SessionName, *SessionSettings);
	}

	return false;
}

void UHostMenuBase::SetupSessionSettings(bool bIsLAN, bool bIsPresence, const int32 &MaxNumPlayers, FName &SessionName, int Rounds) {DebugPrint(__FILE__, __LINE__);
	SessionSettings = MakeShareable(new FOnlineSessionSettings());

	SessionSettings->bIsLANMatch = bIsLAN;
	SessionSettings->bUsesPresence = bIsPresence;
	SessionSettings->NumPublicConnections = MaxNumPlayers;
	SessionSettings->NumPrivateConnections = MaxNumPlayers; // 0;
	SessionSettings->bAllowInvites = true;
	SessionSettings->bAllowJoinInProgress = true;
	SessionSettings->bShouldAdvertise = true;
	SessionSettings->bAllowJoinViaPresence = true;
	SessionSettings->bAllowJoinViaPresenceFriendsOnly = false;

	SessionSettings->Set(SETTING_MAPNAME, LevelToOpen, EOnlineDataAdvertisementType::ViaOnlineService);
	SessionSettings->Set(FName("SessionName"), SessionName.ToString(), EOnlineDataAdvertisementType::ViaOnlineServiceAndPing);
	SessionSettings->Set(FName("Rounds"), Rounds, EOnlineDataAdvertisementType::ViaOnlineServiceAndPing);
}

void UHostMenuBase::OnCreateSessionComplete(FName SessionName, bool bWasSuccessful) {DebugPrint(__FILE__, __LINE__);
	IOnlineSessionPtr Sessions = GetSessionInterface();

	if(Sessions.IsValid()) {DebugPrint(__FILE__, __LINE__);
		Sessions->ClearOnCreateSessionCompleteDelegate_Handle(OnCreateSessionCompleteDelegateHandle);
		if(bWasSuccessful) {DebugPrint(__FILE__, __LINE__);
			OnStartSessionCompleteDelegateHandle = Sessions->AddOnStartSessionCompleteDelegate_Handle(OnStartSessionCompleteDelegate);

			Sessions->StartSession(SessionName);
		}
	}

}

void UHostMenuBase::OnStartOnlineGameComplete(FName SessionName, bool bWasSuccessful) {DebugPrint(__FILE__, __LINE__);
	IOnlineSessionPtr Sessions = GetSessionInterface();
	if(Sessions.IsValid()) {DebugPrint(__FILE__, __LINE__);
		Sessions->ClearOnStartSessionCompleteDelegate_Handle(OnStartSessionCompleteDelegateHandle);
	}

	if(bWasSuccessful) {DebugPrint(__FILE__, __LINE__);
		UGameplayStatics::OpenLevel(GetWorld(), *LevelToOpen, true, "listen");
	}
}