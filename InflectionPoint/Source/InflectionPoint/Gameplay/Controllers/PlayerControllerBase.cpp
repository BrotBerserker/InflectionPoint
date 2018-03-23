// Fill out your copyright notice in the Description page of Project Settings.

#include "InflectionPoint.h"
#include "DebugTools/InflectionPointCheatManager.h"
#include "Gameplay/CharacterInfoProvider.h"
#include "Gameplay/Characters/BaseCharacter.h"
#include "Gameplay/Damage/MortalityProvider.h"
#include "PlayerControllerBase.h"

APlayerControllerBase::APlayerControllerBase(const FObjectInitializer& ObjectInitializer) :
	Super(ObjectInitializer) {DebugPrint(__FILE__, __LINE__);
	CheatClass = UInflectionPointCheatManager::StaticClass();
DebugPrint(__FILE__, __LINE__);}

void APlayerControllerBase::BeginPlay() {DebugPrint(__FILE__, __LINE__);
	Super::BeginPlay();
DebugPrint(__FILE__, __LINE__);}

void APlayerControllerBase::Possess(APawn* InPawn) {DebugPrint(__FILE__, __LINE__);
	Super::Possess(InPawn);

	AssertNotNull(InPawn->PlayerState, GetWorld(), __FILE__, __LINE__);
	GetCharacter()->FindComponentByClass<UCharacterInfoProvider>()->PlayerState = InPawn->PlayerState;
DebugPrint(__FILE__, __LINE__);}

void APlayerControllerBase::ClientSetControlRotation_Implementation(FRotator rotation) {DebugPrint(__FILE__, __LINE__);
	SetControlRotation(rotation);
DebugPrint(__FILE__, __LINE__);}

void APlayerControllerBase::ClientRoundStarted_Implementation(int Round) {DebugPrint(__FILE__, __LINE__);
	OnRoundStarted(Round);
DebugPrint(__FILE__, __LINE__);}

void APlayerControllerBase::ClientShowKillInfo_Implementation(FCharacterInfo KilledInfo, float killedScoreChange, FCharacterInfo KillerInfo, float killerScoreChange, UTexture2D* WeaponImage) {DebugPrint(__FILE__, __LINE__);
	OnKillInfoAdded(KilledInfo, killedScoreChange, KillerInfo, killerScoreChange, WeaponImage);
DebugPrint(__FILE__, __LINE__);}

void APlayerControllerBase::ClientShowCountdownNumber_Implementation(int number) {DebugPrint(__FILE__, __LINE__);
	OnCountdownUpdate(number);
DebugPrint(__FILE__, __LINE__);}

void APlayerControllerBase::ClientSetIgnoreInput_Implementation(bool ignore) {DebugPrint(__FILE__, __LINE__);
	ACharacter* character = GetCharacter();
	AssertNotNull(character, GetWorld(), __FILE__, __LINE__);

	if(ignore) {DebugPrint(__FILE__, __LINE__);
		character->DisableInput(this);
	DebugPrint(__FILE__, __LINE__);} else {DebugPrint(__FILE__, __LINE__);
		character->EnableInput(this);
	DebugPrint(__FILE__, __LINE__);}

DebugPrint(__FILE__, __LINE__);}

bool APlayerControllerBase::ServerSwitchSpectatedCharacter_Validate() {DebugPrint(__FILE__, __LINE__);
	return true;
DebugPrint(__FILE__, __LINE__);}

void APlayerControllerBase::ServerSwitchSpectatedCharacter_Implementation() {DebugPrint(__FILE__, __LINE__);
	TArray<AActor*> foundActors;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), ABaseCharacter::StaticClass(), foundActors);
	int32 myIndex = foundActors.Find(GetViewTarget());

	bool spectatorSwitched = SpectateNextActorInRange(foundActors, myIndex + 1, foundActors.Num());

	if(!spectatorSwitched) {DebugPrint(__FILE__, __LINE__);
		SpectateNextActorInRange(foundActors, 0, myIndex);
	DebugPrint(__FILE__, __LINE__);}
DebugPrint(__FILE__, __LINE__);}

bool APlayerControllerBase::SpectateNextActorInRange(TArray<AActor*> actors, int32 beginIndex, int32 endIndex) {DebugPrint(__FILE__, __LINE__);
	ATDMPlayerStateBase* myPlayerState = Cast<ATDMPlayerStateBase>(PlayerState);
	for(int32 i = beginIndex; i < endIndex; i++) {DebugPrint(__FILE__, __LINE__);
		ABaseCharacter* otherCharacter = Cast<ABaseCharacter>(actors[i]);

		// Don't switch to yourself
		auto character = Cast<ABaseCharacter>(this->GetCharacter());
		AssertNotNull(otherCharacter, GetWorld(), __FILE__, __LINE__);
		AssertNotNull(character, GetWorld(), __FILE__, __LINE__);
		if(character && otherCharacter->GetName().Equals(character->GetName())) {DebugPrint(__FILE__, __LINE__);
			continue;
		DebugPrint(__FILE__, __LINE__);}		

		// Don't switch to current viewtarget
		if(otherCharacter->GetName().Equals(GetViewTarget()->GetName())) {DebugPrint(__FILE__, __LINE__);
			continue;
		DebugPrint(__FILE__, __LINE__);}

		// Don't switch to dead people
		UMortalityProvider* otherMortalityProvider = otherCharacter->FindComponentByClass<UMortalityProvider>();
		if(otherMortalityProvider == NULL || !otherMortalityProvider->IsAlive()) {DebugPrint(__FILE__, __LINE__);
			continue;
		DebugPrint(__FILE__, __LINE__);}

		// Don't switch to players in a different team
		UCharacterInfoProvider* infoProvider = otherCharacter->FindComponentByClass<UCharacterInfoProvider>();
		AssertNotNull(infoProvider, GetWorld(), __FILE__, __LINE__);
		ATDMPlayerStateBase* otherPlayerState = Cast<ATDMPlayerStateBase>(infoProvider->PlayerState);
		AssertNotNull(otherPlayerState, GetWorld(), __FILE__, __LINE__);
		if(otherPlayerState->Team != myPlayerState->Team) {DebugPrint(__FILE__, __LINE__);
			continue;
		DebugPrint(__FILE__, __LINE__);}

		SetViewTargetWithBlend(otherCharacter, 0.3f);
		SpectatedCharacterSwitched(otherCharacter, infoProvider->GetCharacterInfo());
		return true;
	DebugPrint(__FILE__, __LINE__);}

	return false;
DebugPrint(__FILE__, __LINE__);}
