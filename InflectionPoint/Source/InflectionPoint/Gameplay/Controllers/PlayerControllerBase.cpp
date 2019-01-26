// Fill out your copyright notice in the Description page of Project Settings.

#include "InflectionPoint.h"
#include "DebugTools/InflectionPointCheatManager.h"
#include "Gameplay/CharacterInfoProvider.h"
#include "Gameplay/Characters/BaseCharacter.h"
#include "Gameplay/Shop/BaseShopItem.h"
#include "Gameplay/Damage/MortalityProvider.h"
#include "PlayerControllerBase.h"

void APlayerControllerBase::GetLifetimeReplicatedProps(TArray< FLifetimeProperty > & OutLifetimeProps) const {DebugPrint(__FILE__, __LINE__);
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(APlayerControllerBase, SpectatedCharacter);
DebugPrint(__FILE__, __LINE__);}

APlayerControllerBase::APlayerControllerBase(const FObjectInitializer& ObjectInitializer) :
	Super(ObjectInitializer) {DebugPrint(__FILE__, __LINE__);
	PrimaryActorTick.bCanEverTick = true;
	CheatClass = UInflectionPointCheatManager::StaticClass();
DebugPrint(__FILE__, __LINE__);}

void APlayerControllerBase::BeginPlay() {DebugPrint(__FILE__, __LINE__);
	Super::BeginPlay();
DebugPrint(__FILE__, __LINE__);}

void APlayerControllerBase::Tick(float DeltaTime) {DebugPrint(__FILE__, __LINE__);
	UpdateCharactersInLineOfSight();
DebugPrint(__FILE__, __LINE__);}

void APlayerControllerBase::UpdateCharactersInLineOfSight() {DebugPrint(__FILE__, __LINE__);
	TArray<AActor*> foundActors;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), ABaseCharacter::StaticClass(), foundActors);
	TArray<ABaseCharacter*> newArray;
	for(auto& item : foundActors) {DebugPrint(__FILE__, __LINE__);
		auto character = Cast<ABaseCharacter>(item);
		if(character && character->Controller != this && IsLookingAtActor(character))
			newArray.Add(character);		
	DebugPrint(__FILE__, __LINE__);}
	CharactersInLineOfSight = newArray;
DebugPrint(__FILE__, __LINE__);}

void APlayerControllerBase::Possess(APawn* InPawn) {DebugPrint(__FILE__, __LINE__);
	Super::Possess(InPawn);
	SpectatedCharacter = nullptr;
	AssertNotNull(InPawn->GetPlayerState(), GetWorld(), __FILE__, __LINE__);
	GetCharacter()->FindComponentByClass<UCharacterInfoProvider>()->PlayerState = InPawn->GetPlayerState();
DebugPrint(__FILE__, __LINE__);}

void APlayerControllerBase::ClientSetControlRotation_Implementation(FRotator rotation) {DebugPrint(__FILE__, __LINE__);
	SetControlRotation(rotation);
DebugPrint(__FILE__, __LINE__);}

void APlayerControllerBase::ClientPhaseStarted_Implementation(int Phase) {DebugPrint(__FILE__, __LINE__);
	OnPhaseStarted(Phase);
DebugPrint(__FILE__, __LINE__);}

void APlayerControllerBase::ClientShowKillInfo_Implementation(FCharacterInfo KilledInfo, float killedScoreChange, FCharacterInfo KillerInfo, float killerScoreChange, UTexture2D* WeaponImage) {DebugPrint(__FILE__, __LINE__);
	OnKillInfoAdded(KilledInfo, killedScoreChange, KillerInfo, killerScoreChange, WeaponImage);
DebugPrint(__FILE__, __LINE__);}

void APlayerControllerBase::ClientShowPhaseCountdownNumber_Implementation(int number) {DebugPrint(__FILE__, __LINE__);
	OnPhaseCountdownUpdate(number);
DebugPrint(__FILE__, __LINE__);}

void APlayerControllerBase::ClientShowMatchCountdownNumber_Implementation(int number) {DebugPrint(__FILE__, __LINE__);
	OnMatchCountdownUpdate(number);
DebugPrint(__FILE__, __LINE__);}

void APlayerControllerBase::ClientShowShop_Implementation(bool intelligentSmartResetBoolean) {DebugPrint(__FILE__, __LINE__);
	if(intelligentSmartResetBoolean)
		Cast<ATDMPlayerStateBase>(PlayerState)->PrepareForRoundStart(); // "Dies ist geil" - Roman 17.11.2018 nein das stimmt nicht
	OnShowShop(intelligentSmartResetBoolean);
DebugPrint(__FILE__, __LINE__);}

void APlayerControllerBase::ClientShowShopCountdownNumber_Implementation(int number) {DebugPrint(__FILE__, __LINE__);
	OnShopCountdownUpdate(number);
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

void APlayerControllerBase::ClientShowMatchEnd_Implementation(int winnerTeam) {DebugPrint(__FILE__, __LINE__);
	OnShowMatchEnd(winnerTeam);
DebugPrint(__FILE__, __LINE__);}

void APlayerControllerBase::ClientShowRoundEnd_Implementation(int winnerTeam) {DebugPrint(__FILE__, __LINE__);
	OnShowRoundEnd(winnerTeam);
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

		// Don't switch to players in a different team or players without playerstate (e.g. players used for the match end sequence)
		UCharacterInfoProvider* infoProvider = otherCharacter->FindComponentByClass<UCharacterInfoProvider>();
		if(!infoProvider) {DebugPrint(__FILE__, __LINE__);
			continue;
		DebugPrint(__FILE__, __LINE__);}
		ATDMPlayerStateBase* otherPlayerState = Cast<ATDMPlayerStateBase>(infoProvider->PlayerState);
		if(!otherPlayerState || otherPlayerState->Team != myPlayerState->Team) {DebugPrint(__FILE__, __LINE__);
			continue;
		DebugPrint(__FILE__, __LINE__);}

		//UnPossess();
		SetViewTargetWithBlend(otherCharacter, 0.3f);
		SpectatedCharacter = otherCharacter;
		SpectatedCharacterSwitched(otherCharacter, infoProvider->GetCharacterInfo());
		return true;
	DebugPrint(__FILE__, __LINE__);}

	return false;
DebugPrint(__FILE__, __LINE__);}

bool APlayerControllerBase::IsLookingAtActor(AActor* actor, float distance) {DebugPrint(__FILE__, __LINE__);
	if(!actor || !PlayerCameraManager || !GetCharacter() || !this->LineOfSightTo(actor))
		return false;
	FVector b = PlayerCameraManager->GetActorForwardVector(); // looking direction
	FVector a = actor->GetActorLocation() - GetCharacter()->GetActorLocation();
	// p = (<a,b> / <b,b>) * b
	FVector p = (FVector::DotProduct(a, b) / FVector::DotProduct(b, b)) * b;
	float d = (a - p).Size();
	return d <= distance;
DebugPrint(__FILE__, __LINE__);}


bool APlayerControllerBase::ServerPurchaseShopItem_Validate(TSubclassOf<class UBaseShopItem> itemClass) {DebugPrint(__FILE__, __LINE__);
	return true;
DebugPrint(__FILE__, __LINE__);}

void APlayerControllerBase::ServerPurchaseShopItem_Implementation(TSubclassOf<class UBaseShopItem> itemClass) {DebugPrint(__FILE__, __LINE__);
	auto playerState = Cast<ATDMPlayerStateBase>(PlayerState);
	AssertNotNull(playerState, GetWorld(), __FILE__, __LINE__);
	auto item = itemClass.GetDefaultObject();
	AssertNotNull(item, GetWorld(), __FILE__, __LINE__);
	if(!item->IsAffordableForPlayer(playerState))
		return;
	playerState->IPPoints -= item->IPPrice;
	playerState->PurchasedShopItems.Add(item->GetClass());
DebugPrint(__FILE__, __LINE__);}

bool APlayerControllerBase::ServerEquipShopItem_Validate(EInventorySlotPosition inventorySlot, TSubclassOf<class UBaseShopItem> item) {DebugPrint(__FILE__, __LINE__);
	auto playerState = Cast<ATDMPlayerStateBase>(PlayerState);
	AssertNotNull(playerState, GetWorld(), __FILE__, __LINE__);
	return AssertTrue(playerState->PurchasedShopItems.Contains(item), GetWorld(), __FILE__, __LINE__, "Client tries to equip a Shopitem that is not purchased");
DebugPrint(__FILE__, __LINE__);}

void APlayerControllerBase::ServerEquipShopItem_Implementation(EInventorySlotPosition inventorySlot, TSubclassOf<class UBaseShopItem> item) {DebugPrint(__FILE__, __LINE__);
	auto playerState = Cast<ATDMPlayerStateBase>(PlayerState);
	AssertNotNull(playerState, GetWorld(), __FILE__, __LINE__);
	playerState->EquippedShopItems.Add(FTDMEquipSlot(inventorySlot, item));
DebugPrint(__FILE__, __LINE__);}

bool APlayerControllerBase::ServerUnequipShopItemFromSlot_Validate(EInventorySlotPosition slot) {DebugPrint(__FILE__, __LINE__);
	return true;
DebugPrint(__FILE__, __LINE__);}

void APlayerControllerBase::ServerUnequipShopItemFromSlot_Implementation(EInventorySlotPosition slot) {DebugPrint(__FILE__, __LINE__);
	auto playerState = Cast<ATDMPlayerStateBase>(PlayerState);
	AssertNotNull(playerState, GetWorld(), __FILE__, __LINE__);
	for(int i = 0; i < playerState->EquippedShopItems.Num(); i++) {DebugPrint(__FILE__, __LINE__);
		if(playerState->EquippedShopItems[i].Slot == slot) {DebugPrint(__FILE__, __LINE__);
			playerState->EquippedShopItems.RemoveAt(i);
			return;
		DebugPrint(__FILE__, __LINE__);}
	DebugPrint(__FILE__, __LINE__);}
DebugPrint(__FILE__, __LINE__);}