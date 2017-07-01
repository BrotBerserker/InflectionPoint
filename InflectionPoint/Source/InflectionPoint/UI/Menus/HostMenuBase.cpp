// Fill out your copyright notice in the Description page of Project Settings.

#include "InflectionPoint.h"
#include "HostMenuBase.h"

UHostMenuBase::UHostMenuBase() : Super() {
	OnCreateSessionCompleteDelegate = FOnCreateSessionCompleteDelegate::CreateUObject(this, &UHostMenuBase::OnCreateSessionComplete);
	OnStartSessionCompleteDelegate = FOnStartSessionCompleteDelegate::CreateUObject(this, &UHostMenuBase::OnStartOnlineGameComplete);
}

void UHostMenuBase::HostServer(int playerAmount, bool isLan, FString serverName, FString levelToOpen) {
	this->LevelToOpen = levelToOpen;

	ULocalPlayer* const Player = GetWorld()->GetFirstLocalPlayerFromController();

	HostSession(Player->GetPreferredUniqueNetId(), FName(*serverName), isLan, true, playerAmount);
}

IOnlineSessionPtr UHostMenuBase::GetSessionInterface() {
	IOnlineSubsystem* const OnlineSub = IOnlineSubsystem::Get();

	AssertNotNull(OnlineSub, GetWorld(), __FILE__, __LINE__);

	return OnlineSub->GetSessionInterface();
}

bool UHostMenuBase::HostSession(TSharedPtr<const FUniqueNetId> UserId, FName SessionName, bool bIsLAN, bool bIsPresence, int32 MaxNumPlayers) {
	IOnlineSessionPtr Sessions = GetSessionInterface();

	if(Sessions.IsValid() && UserId.IsValid()) {
		SetupSessionSettings(bIsLAN, bIsPresence, MaxNumPlayers, SessionName);

		OnCreateSessionCompleteDelegateHandle = Sessions->AddOnCreateSessionCompleteDelegate_Handle(OnCreateSessionCompleteDelegate);

		return Sessions->CreateSession(*UserId, SessionName, *SessionSettings);
	}

	return false;
}

void UHostMenuBase::SetupSessionSettings(bool bIsLAN, bool bIsPresence, const int32 &MaxNumPlayers, FName &SessionName) {
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
}

void UHostMenuBase::OnCreateSessionComplete(FName SessionName, bool bWasSuccessful) {
	IOnlineSessionPtr Sessions = GetSessionInterface();

	if(Sessions.IsValid()) {
		Sessions->ClearOnCreateSessionCompleteDelegate_Handle(OnCreateSessionCompleteDelegateHandle);
		if(bWasSuccessful) {
			OnStartSessionCompleteDelegateHandle = Sessions->AddOnStartSessionCompleteDelegate_Handle(OnStartSessionCompleteDelegate);

			Sessions->StartSession(SessionName);
		}
	}

}

void UHostMenuBase::OnStartOnlineGameComplete(FName SessionName, bool bWasSuccessful) {
	IOnlineSessionPtr Sessions = GetSessionInterface();
	if(Sessions.IsValid()) {
		Sessions->ClearOnStartSessionCompleteDelegate_Handle(OnStartSessionCompleteDelegateHandle);
	}

	if(bWasSuccessful) {
		UGameplayStatics::OpenLevel(GetWorld(), *LevelToOpen, true, "listen");
	}
}