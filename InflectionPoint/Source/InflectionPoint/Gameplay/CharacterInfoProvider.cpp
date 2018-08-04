// Fill out your copyright notice in the Description page of Project Settings.

#include "InflectionPoint.h"
#include "Gamemodes/TDMPlayerStateBase.h"
#include "Gameplay/Characters/BaseCharacter.h"
#include "Gameplay/Characters/ReplayCharacterBase.h"
#include "CharacterInfoProvider.h"


// Sets default values for this component's properties
UCharacterInfoProvider::UCharacterInfoProvider() {DebugPrint(__FILE__, __LINE__);
	PrimaryComponentTick.bCanEverTick = true;
}

void UCharacterInfoProvider::GetLifetimeReplicatedProps(TArray< FLifetimeProperty > & OutLifetimeProps) const {DebugPrint(__FILE__, __LINE__);
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UCharacterInfoProvider, PlayerState);
}

FCharacterInfo UCharacterInfoProvider::GetCharacterInfo() {DebugPrint(__FILE__, __LINE__);
	// When getting the character info too early (e.g. just after spawn), the playerstate might not be set
	if(PlayerState == NULL) {DebugPrint(__FILE__, __LINE__);
		return FCharacterInfo();
	}
	ATDMPlayerStateBase* tdmPlayerState = Cast<ATDMPlayerStateBase>(PlayerState);
	AReplayCharacterBase* owner = Cast<AReplayCharacterBase>(GetOwner());
	return FCharacterInfo(tdmPlayerState->GetPlayerName(), tdmPlayerState->Team, IsAReplay(), owner ? owner->ReplayIndex : -1);
}

bool UCharacterInfoProvider::IsAReplay() {DebugPrint(__FILE__, __LINE__);
	ABaseCharacter* owner = Cast<ABaseCharacter>(GetOwner());
	return owner->IsAReplay();
}