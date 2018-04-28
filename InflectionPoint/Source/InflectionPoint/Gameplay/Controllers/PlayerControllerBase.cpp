// Fill out your copyright notice in the Description page of Project Settings.

#include "InflectionPoint.h"
#include "DebugTools/InflectionPointCheatManager.h"
#include "Gameplay/CharacterInfoProvider.h"
#include "Gameplay/Characters/BaseCharacter.h"
#include "Gameplay/Damage/MortalityProvider.h"
#include "PlayerControllerBase.h"

void APlayerControllerBase::GetLifetimeReplicatedProps(TArray< FLifetimeProperty > & OutLifetimeProps) const {
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(APlayerControllerBase, SpectatedCharacter);
}

APlayerControllerBase::APlayerControllerBase(const FObjectInitializer& ObjectInitializer) :
	Super(ObjectInitializer) {
	CheatClass = UInflectionPointCheatManager::StaticClass();
}

void APlayerControllerBase::BeginPlay() {
	Super::BeginPlay();
}

void APlayerControllerBase::Possess(APawn* InPawn) {
	Super::Possess(InPawn);
	SpectatedCharacter = nullptr;
	AssertNotNull(InPawn->PlayerState, GetWorld(), __FILE__, __LINE__);
	GetCharacter()->FindComponentByClass<UCharacterInfoProvider>()->PlayerState = InPawn->PlayerState;
	ABaseCharacter* baseCharacter = Cast<ABaseCharacter>(GetCharacter());
	OnWeaponChanged(baseCharacter->CurrentWeapon, NULL);
}

void APlayerControllerBase::ClientSetControlRotation_Implementation(FRotator rotation) {
	SetControlRotation(rotation);
}

void APlayerControllerBase::ClientRoundStarted_Implementation(int Round) {
	OnRoundStarted(Round);
}

void APlayerControllerBase::ClientShowKillInfo_Implementation(FCharacterInfo KilledInfo, float killedScoreChange, FCharacterInfo KillerInfo, float killerScoreChange, UTexture2D* WeaponImage) {
	OnKillInfoAdded(KilledInfo, killedScoreChange, KillerInfo, killerScoreChange, WeaponImage);
}

void APlayerControllerBase::ClientShowCountdownNumber_Implementation(int number) {
	OnCountdownUpdate(number);
}

void APlayerControllerBase::ClientSetIgnoreInput_Implementation(bool ignore) {
	ACharacter* character = GetCharacter();
	AssertNotNull(character, GetWorld(), __FILE__, __LINE__);

	if(ignore) {
		character->DisableInput(this);
	} else {
		character->EnableInput(this);
	}

}

bool APlayerControllerBase::ServerSwitchSpectatedCharacter_Validate() {
	return true;
}

void APlayerControllerBase::ServerSwitchSpectatedCharacter_Implementation() {
	TArray<AActor*> foundActors;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), ABaseCharacter::StaticClass(), foundActors);
	int32 myIndex = foundActors.Find(GetViewTarget());

	bool spectatorSwitched = SpectateNextActorInRange(foundActors, myIndex + 1, foundActors.Num());

	if(!spectatorSwitched) {
		SpectateNextActorInRange(foundActors, 0, myIndex);
	}
}

bool APlayerControllerBase::SpectateNextActorInRange(TArray<AActor*> actors, int32 beginIndex, int32 endIndex) {
	ATDMPlayerStateBase* myPlayerState = Cast<ATDMPlayerStateBase>(PlayerState);
	for(int32 i = beginIndex; i < endIndex; i++) {
		ABaseCharacter* otherCharacter = Cast<ABaseCharacter>(actors[i]);

		// Don't switch to yourself
		auto character = Cast<ABaseCharacter>(this->GetCharacter());
		AssertNotNull(otherCharacter, GetWorld(), __FILE__, __LINE__);
		AssertNotNull(character, GetWorld(), __FILE__, __LINE__);
		if(character && otherCharacter->GetName().Equals(character->GetName())) {
			continue;
		}		

		// Don't switch to current viewtarget
		if(otherCharacter->GetName().Equals(GetViewTarget()->GetName())) {
			continue;
		}

		// Don't switch to dead people
		UMortalityProvider* otherMortalityProvider = otherCharacter->FindComponentByClass<UMortalityProvider>();
		if(otherMortalityProvider == NULL || !otherMortalityProvider->IsAlive()) {
			continue;
		}

		// Don't switch to players in a different team
		UCharacterInfoProvider* infoProvider = otherCharacter->FindComponentByClass<UCharacterInfoProvider>();
		AssertNotNull(infoProvider, GetWorld(), __FILE__, __LINE__);
		ATDMPlayerStateBase* otherPlayerState = Cast<ATDMPlayerStateBase>(infoProvider->PlayerState);
		AssertNotNull(otherPlayerState, GetWorld(), __FILE__, __LINE__);
		if(otherPlayerState->Team != myPlayerState->Team) {
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