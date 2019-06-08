// Fill out your copyright notice in the Description page of Project Settings.

#include "InflectionPoint.h"
#include "Gameplay/Characters/BaseCharacter.h"
#include "PlayerStateRecorder.h"

// Sets default values for this component's properties
UPlayerStateRecorder::UPlayerStateRecorder() {
	PrimaryComponentTick.bCanEverTick = true;
}

void UPlayerStateRecorder::InitializeBindings(UInputComponent * inputComponent) {
	inputComponent->BindAction("Reload", IE_Pressed, this, &UPlayerStateRecorder::RecordReload<IE_Pressed>);
	inputComponent->BindAction("Reload", IE_Released, this, &UPlayerStateRecorder::RecordReload<IE_Released>);

	inputComponent->BindAction("NextWeapon", IE_Pressed, this, &UPlayerStateRecorder::RecordEquipNextWeapon<IE_Pressed>);
	inputComponent->BindAction("NextWeapon", IE_Released, this, &UPlayerStateRecorder::RecordEquipNextWeapon<IE_Released>);

	inputComponent->BindAction("PreviousWeapon", IE_Pressed, this, &UPlayerStateRecorder::RecordEquipPreviousWeapon<IE_Pressed>);
	inputComponent->BindAction("PreviousWeapon", IE_Released, this, &UPlayerStateRecorder::RecordEquipPreviousWeapon<IE_Released>);

	inputComponent->BindAction("SwitchToWeapon1", IE_Pressed, this, &UPlayerStateRecorder::RecordEquipSpecificSlot<EInventorySlotPosition::Weapon1, IE_Pressed>);
	inputComponent->BindAction("SwitchToWeapon1", IE_Released, this, &UPlayerStateRecorder::RecordEquipSpecificSlot<EInventorySlotPosition::Weapon1, IE_Released>);
	inputComponent->BindAction("SwitchToWeapon2", IE_Pressed, this, &UPlayerStateRecorder::RecordEquipSpecificSlot<EInventorySlotPosition::Weapon2, IE_Pressed>);
	inputComponent->BindAction("SwitchToWeapon2", IE_Released, this, &UPlayerStateRecorder::RecordEquipSpecificSlot<EInventorySlotPosition::Weapon2, IE_Released>);
	inputComponent->BindAction("SwitchToWeapon3", IE_Pressed, this, &UPlayerStateRecorder::RecordEquipSpecificSlot<EInventorySlotPosition::Weapon3, IE_Pressed>);
	inputComponent->BindAction("SwitchToWeapon3", IE_Released, this, &UPlayerStateRecorder::RecordEquipSpecificSlot<EInventorySlotPosition::Weapon3, IE_Released>);
	inputComponent->BindAction("SwitchToSkill1", IE_Pressed, this, &UPlayerStateRecorder::RecordEquipSpecificSlot<EInventorySlotPosition::Skill1, IE_Pressed>);
	inputComponent->BindAction("SwitchToSkill1", IE_Released, this, &UPlayerStateRecorder::RecordEquipSpecificSlot<EInventorySlotPosition::Skill1, IE_Released>);
	inputComponent->BindAction("SwitchToSkill2", IE_Pressed, this, &UPlayerStateRecorder::RecordEquipSpecificSlot<EInventorySlotPosition::Skill2, IE_Pressed>);
	inputComponent->BindAction("SwitchToSkill2", IE_Released, this, &UPlayerStateRecorder::RecordEquipSpecificSlot<EInventorySlotPosition::Skill2, IE_Released>);
	inputComponent->BindAction("SwitchToSkill3", IE_Pressed, this, &UPlayerStateRecorder::RecordEquipSpecificSlot<EInventorySlotPosition::Skill3, IE_Pressed>);
	inputComponent->BindAction("SwitchToSkill3", IE_Released, this, &UPlayerStateRecorder::RecordEquipSpecificSlot<EInventorySlotPosition::Skill3, IE_Released>);

	inputComponent->BindAction("Jump", IE_Pressed, this, &UPlayerStateRecorder::RecordJump<IE_Pressed>);
	inputComponent->BindAction("Jump", IE_Released, this, &UPlayerStateRecorder::RecordJump<IE_Released>);

	inputComponent->BindAction("Crouch", IE_Pressed, this, &UPlayerStateRecorder::RecordToggleCrouching<IE_Pressed>);

	inputComponent->BindAction("Aim", IE_Pressed, this, &UPlayerStateRecorder::RecordAim<IE_Pressed>);
	inputComponent->BindAction("Aim", IE_Released, this, &UPlayerStateRecorder::RecordAim<IE_Released>);

	inputComponent->BindAction("Sprint", IE_Pressed, this, &UPlayerStateRecorder::RecordSprint<IE_Pressed>);
	inputComponent->BindAction("Sprint", IE_Released, this, &UPlayerStateRecorder::RecordSprint<IE_Released>);

	inputComponent->BindAxis("MoveForward", this, &UPlayerStateRecorder::RecordMoveForward);
	inputComponent->BindAxis("MoveRight", this, &UPlayerStateRecorder::RecordMoveRight);
}


// Called when the game starts
void UPlayerStateRecorder::BeginPlay() {
	Super::BeginPlay();

	//passedTime = 0.f;

	owner = (ABaseCharacter*)GetOwner();
	AssertNotNull(owner, GetWorld(), __FILE__, __LINE__);
}


// Called every frame
void UPlayerStateRecorder::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) {
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if(!recording) {
		return;
	}

	passedTime += DeltaTime;

	// Get data to be recorded
	FVector pos = owner->GetTransform().GetLocation();
	float yaw = owner->GetCapsuleComponent()->GetComponentRotation().Yaw;
	float pitch = owner->FirstPersonCameraComponent->GetComponentRotation().Pitch;

	RecordedPlayerStates.Add(FRecordedPlayerState(passedTime, pos, yaw, pitch, pressedKeys, releasedKeys));
	pressedKeys = TArray<FString>();
	releasedKeys = TArray<FString>();
}

bool UPlayerStateRecorder::ServerStartRecording_Validate() {
	return true;
}

void UPlayerStateRecorder::ServerStartRecording_Implementation() {
	ServerResetRecordedPlayerStates();
	recordedPlayerStateQueue.Empty();
	passedTime = 0.f;
	recording = true;
}

bool UPlayerStateRecorder::ServerResetRecordedPlayerStates_Validate() {
	return true;
}

void UPlayerStateRecorder::ServerResetRecordedPlayerStates_Implementation() {
	RecordedPlayerStates.Empty();
	pressedKeys.Empty();
	releasedKeys.Empty();
}

bool UPlayerStateRecorder::ServerRecordKeyPressed_Validate(const FString &key) {
	return true;
}

void UPlayerStateRecorder::ServerRecordKeyPressed_Implementation(const FString &key) {
	pressedKeys.Add(key);
}

bool UPlayerStateRecorder::ServerRecordKeyReleased_Validate(const FString &key) {
	return true;
}

void UPlayerStateRecorder::ServerRecordKeyReleased_Implementation(const FString &key) {
	releasedKeys.Add(key);
}

void UPlayerStateRecorder::RecordFirePressed(EFireMode mode) {
	RecordKey(FString("FireWithMode") + FString::FromInt((int)mode), EInputEvent::IE_Pressed);
}

void UPlayerStateRecorder::RecordFireReleased(EFireMode mode) {
	RecordKey(FString("FireWithMode") + FString::FromInt((int)mode), EInputEvent::IE_Released);
}

void UPlayerStateRecorder::RecordMoveForward(float val) {
	if(movingForward == val) {
		return;
	}

	if(val > 0) {
		ServerRecordKeyPressed("MoveForward");
	} else if(val < 0) {
		ServerRecordKeyPressed("MoveBackward");
	}

	if(movingForward > 0) {
		ServerRecordKeyReleased("MoveForward");
	} else if(movingForward < 0) {
		ServerRecordKeyReleased("MoveBackward");
	}

	movingForward = val;
}

void UPlayerStateRecorder::RecordMoveRight(float val) {
	if(movingRight == val) {
		return;
	}

	if(val > 0) {
		ServerRecordKeyPressed("MoveRight");
	} else if(val < 0) {
		ServerRecordKeyPressed("MoveLeft");
	}

	if(movingRight > 0) {
		ServerRecordKeyReleased("MoveRight");
	} else if(movingRight < 0) {
		ServerRecordKeyReleased("MoveLeft");
	}

	movingRight = val;
}

void UPlayerStateRecorder::RecordKey(FString key, EInputEvent eventType) {
	if(eventType == IE_Pressed)
		ServerRecordKeyPressed(key);
	if(eventType == IE_Released)
		ServerRecordKeyReleased(key);
}

template<EInputEvent eventType>
void UPlayerStateRecorder::RecordJump() {
	RecordKey("Jump", eventType);
}

template<EInputEvent eventType>
void UPlayerStateRecorder::RecordToggleCrouching() {
	RecordKey("ToggleCrouching", eventType);
}

template<EInputEvent eventType>
void UPlayerStateRecorder::RecordSprint() {
	RecordKey("Sprint", eventType);
}

template<EInputEvent eventType>
void UPlayerStateRecorder::RecordAim() {
	RecordKey("Aim", eventType);
}

template<EInputEvent eventType>
void UPlayerStateRecorder::RecordReload() {
	RecordKey("Reload", eventType);
}

template<EInputEvent eventType>
void UPlayerStateRecorder::RecordEquipNextWeapon() {
	RecordKey("EquipNextWeapon", eventType);
}

template<EInputEvent eventType>
void UPlayerStateRecorder::RecordEquipPreviousWeapon() {
	RecordKey("EquipPreviousWeapon", eventType);
}

template<EInventorySlotPosition slot, EInputEvent eventType>
void UPlayerStateRecorder::RecordEquipSpecificSlot() {
	RecordKey(FString("EquipSpecificSlot") + FString::FromInt((int)slot), eventType);
}
