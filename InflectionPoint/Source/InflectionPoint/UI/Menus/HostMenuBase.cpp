// Fill out your copyright notice in the Description page of Project Settings.

#include "InflectionPoint.h"
#include "HostMenuBase.h"

void UHostMenuBase::HostServer(int playerAmount, bool isLan, FString serverName) {
	// Creating a local player where we can get the UserID from
	ULocalPlayer* const Player = GetWorld()->GetFirstLocalPlayerFromController();// GetFirstGamePlayer();

	// Call our custom HostSession function. GameSessionName is a GameInstance variable
	HostSession(Player->GetPreferredUniqueNetId(), GameSessionName/*FName(*serverName)*/, isLan, true, playerAmount);
}

UHostMenuBase::UHostMenuBase()
	: Super() {
	/** Bind function for CREATING a Session */
	OnCreateSessionCompleteDelegate = FOnCreateSessionCompleteDelegate::CreateUObject(this, &UHostMenuBase::OnCreateSessionComplete);
	OnStartSessionCompleteDelegate = FOnStartSessionCompleteDelegate::CreateUObject(this, &UHostMenuBase::OnStartOnlineGameComplete);
}

bool UHostMenuBase::HostSession(TSharedPtr<const FUniqueNetId> UserId, FName SessionName, bool bIsLAN, bool bIsPresence, int32 MaxNumPlayers) {
	// Get the Online Subsystem to work with
	IOnlineSubsystem* const OnlineSub = IOnlineSubsystem::Get();

	if(OnlineSub) {
		// Get the Session Interface, so we can call the "CreateSession" function on it
		IOnlineSessionPtr Sessions = OnlineSub->GetSessionInterface();

		if(Sessions.IsValid() && UserId.IsValid()) {
			/*
			Fill in all the Session Settings that we want to use.

			There are more with SessionSettings.Set(...);
			For example the Map or the GameMode/Type.
			*/
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

			SessionSettings->Set(SETTING_MAPNAME, NameOfLevelToOpen/*FString("NewMap")*/, EOnlineDataAdvertisementType::ViaOnlineService);

			// Set the delegate to the Handle of the SessionInterface
			OnCreateSessionCompleteDelegateHandle = Sessions->AddOnCreateSessionCompleteDelegate_Handle(OnCreateSessionCompleteDelegate);

			// Our delegate should get called when this is complete (doesn't need to be successful!)
			return Sessions->CreateSession(*UserId, SessionName, *SessionSettings);
		}
	} else {
		GEngine->AddOnScreenDebugMessage(-1, 100.f, FColor::Red, TEXT("No OnlineSubsytem found!"));
	}

	return false;
}

void UHostMenuBase::OnCreateSessionComplete(FName SessionName, bool bWasSuccessful) {
	GEngine->AddOnScreenDebugMessage(-1, 100.f, FColor::Red, FString::Printf(TEXT("OnCreateSessionComplete %s, %d"), *SessionName.ToString(), bWasSuccessful));

	// Get the OnlineSubsystem so we can get the Session Interface
	IOnlineSubsystem* OnlineSub = IOnlineSubsystem::Get();
	if(OnlineSub) {
		// Get the Session Interface to call the StartSession function
		IOnlineSessionPtr Sessions = OnlineSub->GetSessionInterface();

		if(Sessions.IsValid()) {
			// Clear the SessionComplete delegate handle, since we finished this call
			Sessions->ClearOnCreateSessionCompleteDelegate_Handle(OnCreateSessionCompleteDelegateHandle);
			if(bWasSuccessful) {
				// Set the StartSession delegate handle
				OnStartSessionCompleteDelegateHandle = Sessions->AddOnStartSessionCompleteDelegate_Handle(OnStartSessionCompleteDelegate);

				// Our StartSessionComplete delegate should get called after this
				Sessions->StartSession(SessionName);
			}
		}

	}
}

void UHostMenuBase::OnStartOnlineGameComplete(FName SessionName, bool bWasSuccessful) {
	GEngine->AddOnScreenDebugMessage(-1, 100.f, FColor::Red, FString::Printf(TEXT("OnStartSessionComplete %s, %d"), *SessionName.ToString(), bWasSuccessful));

	// Get the Online Subsystem so we can get the Session Interface
	IOnlineSubsystem* OnlineSub = IOnlineSubsystem::Get();
	if(OnlineSub) {
		// Get the Session Interface to clear the Delegate
		IOnlineSessionPtr Sessions = OnlineSub->GetSessionInterface();
		if(Sessions.IsValid()) {
			// Clear the delegate, since we are done with this call
			Sessions->ClearOnStartSessionCompleteDelegate_Handle(OnStartSessionCompleteDelegateHandle);
		}
	}

	// If the start was successful, we can open a NewMap if we want. Make sure to use "listen" as a parameter!
	if(bWasSuccessful) {
		UE_LOG(LogTemp, Warning, TEXT("Opening map now"));
		UGameplayStatics::OpenLevel(GetWorld(), *NameOfLevelToOpen, true, "listen");
	}
}