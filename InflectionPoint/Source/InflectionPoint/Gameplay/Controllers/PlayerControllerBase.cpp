// Fill out your copyright notice in the Description page of Project Settings.

#include "InflectionPoint.h"
#include "DebugTools/InflectionPointCheatManager.h"
#include "Gameplay/CharacterInfoProvider.h"
#include "Gameplay/Characters/BaseCharacter.h"
#include "Gameplay/Damage/MortalityProvider.h"
#include "PlayerControllerBase.h"

void APlayerControllerBase::GetLifetimeReplicatedProps(TArray< FLifetimeProperty > & OutLifetimeProps) const {DebugPrint(__FILE__, __LINE__);
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(APlayerControllerBase, SpectatedCharacter);
}

APlayerControllerBase::APlayerControllerBase(const FObjectInitializer& ObjectInitializer) :
	Super(ObjectInitializer) {DebugPrint(__FILE__, __LINE__);
	CheatClass = UInflectionPointCheatManager::StaticClass();
}

void APlayerControllerBase::BeginPlay() {DebugPrint(__FILE__, __LINE__);
	Super::BeginPlay();
}

void APlayerControllerBase::Possess(APawn* InPawn) {DebugPrint(__FILE__, __LINE__);
	Super::Possess(InPawn);
	SpectatedCharacter = nullptr;
	AssertNotNull(InPawn->PlayerState, GetWorld(), __FILE__, __LINE__);
	GetCharacter()->FindComponentByClass<UCharacterInfoProvider>()->PlayerState = InPawn->PlayerState;
}

void APlayerControllerBase::ClientSetControlRotation_Implementation(FRotator rotation) {DebugPrint(__FILE__, __LINE__);
	SetControlRotation(rotation);
}

void APlayerControllerBase::ClientPhaseStarted_Implementation(int Phase) {DebugPrint(__FILE__, __LINE__);
	OnPhaseStarted(Phase);
}

void APlayerControllerBase::ClientShowKillInfo_Implementation(FCharacterInfo KilledInfo, float killedScoreChange, FCharacterInfo KillerInfo, float killerScoreChange, UTexture2D* WeaponImage) {DebugPrint(__FILE__, __LINE__);
	OnKillInfoAdded(KilledInfo, killedScoreChange, KillerInfo, killerScoreChange, WeaponImage);
}

void APlayerControllerBase::ClientShowCountdownNumber_Implementation(int number) {DebugPrint(__FILE__, __LINE__);
	OnCountdownUpdate(number);
}

void APlayerControllerBase::ClientSetIgnoreInput_Implementation(bool ignore) {DebugPrint(__FILE__, __LINE__);
	ACharacter* character = GetCharacter();
	AssertNotNull(character, GetWorld(), __FILE__, __LINE__);

	if(ignore) {DebugPrint(__FILE__, __LINE__);
		character->DisableInput(this);
	} else {DebugPrint(__FILE__, __LINE__);
		character->EnableInput(this);
	}

}

void APlayerControllerBase::ClientShowMatchEnd_Implementation(int winnerTeam) {DebugPrint(__FILE__, __LINE__);
	OnShowMatchEnd(winnerTeam);
}

bool APlayerControllerBase::ServerSwitchSpectatedCharacter_Validate() {DebugPrint(__FILE__, __LINE__);
	return true;
}

void APlayerControllerBase::ServerSwitchSpectatedCharacter_Implementation() {DebugPrint(__FILE__, __LINE__);
	TArray<AActor*> foundActors;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), ABaseCharacter::StaticClass(), foundActors);
	int32 myIndex = foundActors.Find(GetViewTarget());

	bool spectatorSwitched = SpectateNextActorInRange(foundActors, myIndex + 1, foundActors.Num());

	if(!spectatorSwitched) {DebugPrint(__FILE__, __LINE__);
		SpectateNextActorInRange(foundActors, 0, myIndex);
	}
}

bool APlayerControllerBase::SpectateNextActorInRange(TArray<AActor*> actors, int32 beginIndex, int32 endIndex) {DebugPrint(__FILE__, __LINE__);
	ATDMPlayerStateBase* myPlayerState = Cast<ATDMPlayerStateBase>(PlayerState);
	for(int32 i = beginIndex; i < endIndex; i++) {DebugPrint(__FILE__, __LINE__);
		ABaseCharacter* otherCharacter = Cast<ABaseCharacter>(actors[i]);

		// Don't switch to yourself
		auto character = Cast<ABaseCharacter>(this->GetCharacter());
		AssertNotNull(otherCharacter, GetWorld(), __FILE__, __LINE__);
		if(character && otherCharacter->GetName().Equals(character->GetName())) {DebugPrint(__FILE__, __LINE__);
			continue;
		}		

		// Don't switch to current viewtarget
		if(otherCharacter->GetName().Equals(GetViewTarget()->GetName())) {DebugPrint(__FILE__, __LINE__);
			continue;
		}

		// Don't switch to dead people
		UMortalityProvider* otherMortalityProvider = otherCharacter->FindComponentByClass<UMortalityProvider>();
		if(otherMortalityProvider == NULL || !otherMortalityProvider->IsAlive()) {DebugPrint(__FILE__, __LINE__);
			continue;
		}

		// Don't switch to players in a different team or players without playerstate (e.g. players used for the match end sequence)
		UCharacterInfoProvider* infoProvider = otherCharacter->FindComponentByClass<UCharacterInfoProvider>();
		if(!infoProvider) {DebugPrint(__FILE__, __LINE__);
			continue;
		}
		ATDMPlayerStateBase* otherPlayerState = Cast<ATDMPlayerStateBase>(infoProvider->PlayerState);
		if(!otherPlayerState || otherPlayerState->Team != myPlayerState->Team) {DebugPrint(__FILE__, __LINE__);
			continue;
		}

		//UnPossess();
		SetViewTargetWithBlend(otherCharacter, 0.3f);
		SpectatedCharacter = otherCharacter;
		SpectatedCharacterSwitched(otherCharacter, infoProvider->GetCharacterInfo());
		return true;
	}

	return false;
}