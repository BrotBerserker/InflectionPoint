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
	PrimaryActorTick.bCanEverTick = true;
	CheatClass = UInflectionPointCheatManager::StaticClass();
}

void APlayerControllerBase::BeginPlay() {
	Super::BeginPlay();
}

void APlayerControllerBase::Tick(float DeltaTime) {
	UpdateCharactersInLineOfSight();
}

void APlayerControllerBase::UpdateCharactersInLineOfSight() {
	TArray<AActor*> foundActors;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), ABaseCharacter::StaticClass(), foundActors);
	TArray<ABaseCharacter*> newArray;
	for(auto& item : foundActors) {
		auto character = Cast<ABaseCharacter>(item);
		if(character && character->Controller != this && IsLookingAtActor(character))
			newArray.Add(character);		
	}
	CharactersInLineOfSight = newArray;
}

ABaseCharacter* APlayerControllerBase::GetNearestAliveCharacterInLineOfSight() {
	if(CharactersInLineOfSight.Num() <= 0 || !PlayerCameraManager)
		return nullptr;
	ABaseCharacter* nearestCharacter = NULL;
	float bestDistance = FLT_MAX;
	for(auto& item : CharactersInLineOfSight) {
		float dist = (item->GetActorLocation() - PlayerCameraManager->GetCameraLocation()).Size();
		if(!item->IsAlive() || dist >= bestDistance)
			continue;
		bestDistance = dist;
		nearestCharacter = item;
	}
	return nearestCharacter;
}

void APlayerControllerBase::Possess(APawn* InPawn) {
	Super::Possess(InPawn);
	SpectatedCharacter = nullptr;
	AssertNotNull(InPawn->PlayerState, GetWorld(), __FILE__, __LINE__);
	GetCharacter()->FindComponentByClass<UCharacterInfoProvider>()->PlayerState = InPawn->PlayerState;
}

void APlayerControllerBase::ClientSetControlRotation_Implementation(FRotator rotation) {
	SetControlRotation(rotation);
}

void APlayerControllerBase::ClientPhaseStarted_Implementation(int Phase) {
	OnPhaseStarted(Phase);
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

void APlayerControllerBase::ClientShowMatchEnd_Implementation(int winnerTeam) {
	OnShowMatchEnd(winnerTeam);
}

void APlayerControllerBase::ClientShowRoundEnd_Implementation(int winnerTeam) {
	OnShowRoundEnd(winnerTeam);
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

		// Don't switch to players in a different team or players without playerstate (e.g. players used for the match end sequence)
		UCharacterInfoProvider* infoProvider = otherCharacter->FindComponentByClass<UCharacterInfoProvider>();
		if(!infoProvider) {
			continue;
		}
		ATDMPlayerStateBase* otherPlayerState = Cast<ATDMPlayerStateBase>(infoProvider->PlayerState);
		if(!otherPlayerState || otherPlayerState->Team != myPlayerState->Team) {
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

bool APlayerControllerBase::IsLookingAtActor(AActor* actor, float distance) {
	if(!actor || !PlayerCameraManager || !GetCharacter() || !this->LineOfSightTo(actor))
		return false;
	FVector b = PlayerCameraManager->GetActorForwardVector(); // looking direction
	FVector a = actor->GetActorLocation() - GetCharacter()->GetActorLocation();
	// p = (<a,b> / <b,b>) * b
	FVector p = (FVector::DotProduct(a, b) / FVector::DotProduct(b, b)) * b;
	float d = (a - p).Size();
	return d <= distance;
}