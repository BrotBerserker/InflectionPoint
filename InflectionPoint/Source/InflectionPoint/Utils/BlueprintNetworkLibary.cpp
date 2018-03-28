// Fill out your copyright notice in the Description page of Project Settings.

#include "InflectionPoint.h"
#include "BlueprintNetworkLibary.h"

UBlueprintNetworkLibary::UBlueprintNetworkLibary(class FObjectInitializer const & Initializer) : Super(Initializer) {
}

void UBlueprintNetworkLibary::LeaveMultiplayerGame(FName SessionName) { //test
	IOnlineSubsystem* OnlineSub = IOnlineSubsystem::Get();
	if(OnlineSub) {
		IOnlineSessionPtr Sessions = OnlineSub->GetSessionInterface();
		if(Sessions.IsValid()) {
			//Sessions->AddOnDestroySessionCompleteDelegate_Handle(OnDestroySessionCompleteDelegate);

			Sessions->DestroySession(SessionName);
		}
	}
}