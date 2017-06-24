// Fill out your copyright notice in the Description page of Project Settings.

#include "InflectionPoint.h"
#include "BluprintNetworkLibary.h"


UBluprintNetworkLibary::UBluprintNetworkLibary() {
	OnDestroySessionCompleteDelegate = FOnDestroySessionCompleteDelegate::CreateUObject(this, &UBluprintNetworkLibary::OnDestroySessionComplete);
}

void UBluprintNetworkLibary::OnDestroySessionComplete(FName SessionName, bool bWasSuccessful) {
	//GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Red, FString::Printf(TEXT("OnDestroySessionComplete %s, %d"), *SessionName.ToString(), bWasSuccessful));

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

void UBluprintNetworkLibary::LeaveMultiplayerGame(FName SessionName) {
	IOnlineSubsystem* OnlineSub = IOnlineSubsystem::Get();
	if(OnlineSub) {
		IOnlineSessionPtr Sessions = OnlineSub->GetSessionInterface();
		if(Sessions.IsValid()) {
			Sessions->AddOnDestroySessionCompleteDelegate_Handle(OnDestroySessionCompleteDelegate);

			Sessions->DestroySession(SessionName);
		}
	}
}