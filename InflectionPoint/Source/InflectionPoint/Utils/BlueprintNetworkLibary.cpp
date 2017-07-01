// Fill out your copyright notice in the Description page of Project Settings.

#include "InflectionPoint.h"
#include "BlueprintNetworkLibary.h"

UBlueprintNetworkLibary::UBlueprintNetworkLibary() {
	OnDestroySessionCompleteDelegate = FOnDestroySessionCompleteDelegate::CreateUObject(this, &UBlueprintNetworkLibary::OnDestroySessionComplete);
}

void UBlueprintNetworkLibary::OnDestroySessionComplete(FName SessionName, bool bWasSuccessful) {
	// Get the OnlineSubsystem we want to work with
	IOnlineSubsystem* OnlineSub = IOnlineSubsystem::Get();
	if(OnlineSub) {
		// Get the SessionInterface from the OnlineSubsystem
		IOnlineSessionPtr Sessions = OnlineSub->GetSessionInterface();

		if(Sessions.IsValid()) {
			// Clear the Delegate
			Sessions->ClearOnDestroySessionCompleteDelegate_Handle(OnDestroySessionCompleteDelegateHandle);

			if(bWasSuccessful) {
				OnSessionDestroyed();
			}
		}
	}
}

void UBlueprintNetworkLibary::LeaveMultiplayerGame(FName SessionName) { //test
	IOnlineSubsystem* OnlineSub = IOnlineSubsystem::Get();
	if(OnlineSub) {
		IOnlineSessionPtr Sessions = OnlineSub->GetSessionInterface();
		if(Sessions.IsValid()) {
			Sessions->AddOnDestroySessionCompleteDelegate_Handle(OnDestroySessionCompleteDelegate);

			Sessions->DestroySession(SessionName);
		}
	}
}