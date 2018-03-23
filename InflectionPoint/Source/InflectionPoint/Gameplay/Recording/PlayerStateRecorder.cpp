// Fill out your copyright notice in the Description page of Project Settings.

#include "InflectionPoint.h"
#include "PlayerStateRecorder.h"


// Sets default values for this component's properties
UPlayerStateRecorder::UPlayerStateRecorder() {DebugPrint(__FILE__, __LINE__);
	PrimaryComponentTick.bCanEverTick = true;
DebugPrint(__FILE__, __LINE__);}

void UPlayerStateRecorder::InitializeBindings(UInputComponent * inputComponent) {DebugPrint(__FILE__, __LINE__);
	inputComponent->BindAction("Reload", IE_Pressed, this, &UPlayerStateRecorder::RecordReload<IE_Pressed>);
	inputComponent->BindAction("Reload", IE_Released, this, &UPlayerStateRecorder::RecordReload<IE_Released>);

	inputComponent->BindAction("NextWeapon", IE_Pressed, this, &UPlayerStateRecorder::RecordEquipNextWeapon<IE_Pressed>);
	inputComponent->BindAction("NextWeapon", IE_Released, this, &UPlayerStateRecorder::RecordEquipNextWeapon<IE_Released>);

	inputComponent->BindAction("PreviousWeapon", IE_Pressed, this, &UPlayerStateRecorder::RecordEquipPreviousWeapon<IE_Pressed>);
	inputComponent->BindAction("PreviousWeapon", IE_Released, this, &UPlayerStateRecorder::RecordEquipPreviousWeapon<IE_Released>);

	inputComponent->BindAction("SwitchToWeapon1", IE_Pressed, this, &UPlayerStateRecorder::RecordEquipSpecificWeapon<0, IE_Pressed>);
	inputComponent->BindAction("SwitchToWeapon1", IE_Released, this, &UPlayerStateRecorder::RecordEquipSpecificWeapon<0, IE_Released>);
	inputComponent->BindAction("SwitchToWeapon2", IE_Pressed, this, &UPlayerStateRecorder::RecordEquipSpecificWeapon<1, IE_Pressed>);
	inputComponent->BindAction("SwitchToWeapon2", IE_Released, this, &UPlayerStateRecorder::RecordEquipSpecificWeapon<1, IE_Released>);
	inputComponent->BindAction("SwitchToWeapon3", IE_Pressed, this, &UPlayerStateRecorder::RecordEquipSpecificWeapon<2, IE_Pressed>);
	inputComponent->BindAction("SwitchToWeapon3", IE_Released, this, &UPlayerStateRecorder::RecordEquipSpecificWeapon<2, IE_Released>);
	inputComponent->BindAction("SwitchToWeapon4", IE_Pressed, this, &UPlayerStateRecorder::RecordEquipSpecificWeapon<3, IE_Pressed>);
	inputComponent->BindAction("SwitchToWeapon4", IE_Released, this, &UPlayerStateRecorder::RecordEquipSpecificWeapon<3, IE_Released>);
	inputComponent->BindAction("SwitchToWeapon5", IE_Pressed, this, &UPlayerStateRecorder::RecordEquipSpecificWeapon<4, IE_Pressed>);
	inputComponent->BindAction("SwitchToWeapon5", IE_Released, this, &UPlayerStateRecorder::RecordEquipSpecificWeapon<4, IE_Released>);

	inputComponent->BindAction("Jump", IE_Pressed, this, &UPlayerStateRecorder::RecordJump<IE_Pressed>);
	inputComponent->BindAction("Jump", IE_Released, this, &UPlayerStateRecorder::RecordJump<IE_Released>);

	inputComponent->BindAction("Aim", IE_Pressed, this, &UPlayerStateRecorder::RecordAim<IE_Pressed>);
	inputComponent->BindAction("Aim", IE_Released, this, &UPlayerStateRecorder::RecordAim<IE_Released>);

	inputComponent->BindAction("Sprint", IE_Pressed, this, &UPlayerStateRecorder::RecordSprint<IE_Pressed>);
	inputComponent->BindAction("Sprint", IE_Released, this, &UPlayerStateRecorder::RecordSprint<IE_Released>);

	inputComponent->BindAxis("MoveForward", this, &UPlayerStateRecorder::RecordMoveForward);
	inputComponent->BindAxis("MoveRight", this, &UPlayerStateRecorder::RecordMoveRight);
DebugPrint(__FILE__, __LINE__);}


// Called when the game starts
void UPlayerStateRecorder::BeginPlay() {DebugPrint(__FILE__, __LINE__);
	Super::BeginPlay();

	//passedTime = 0.f;

	owner = (ABaseCharacter*)GetOwner();
	AssertNotNull(owner, GetWorld(), __FILE__, __LINE__);
DebugPrint(__FILE__, __LINE__);}


// Called every frame
void UPlayerStateRecorder::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) {DebugPrint(__FILE__, __LINE__);
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if(!recording) {DebugPrint(__FILE__, __LINE__);
		return;
	DebugPrint(__FILE__, __LINE__);}

	passedTime += DeltaTime;

	// Get data to be recorded
	FVector pos = owner->GetTransform().GetLocation();
	float yaw = owner->GetCapsuleComponent()->GetComponentRotation().Yaw;
	float pitch = owner->FirstPersonCameraComponent->GetComponentRotation().Pitch;

	RecordedPlayerStates.Add(FRecordedPlayerState(passedTime, pos, yaw, pitch, pressedKeys, releasedKeys));
	pressedKeys = TArray<FString>();
	releasedKeys = TArray<FString>();
DebugPrint(__FILE__, __LINE__);}

bool UPlayerStateRecorder::ServerStartRecording_Validate() {DebugPrint(__FILE__, __LINE__);
	return true;
DebugPrint(__FILE__, __LINE__);}

void UPlayerStateRecorder::ServerStartRecording_Implementation() {DebugPrint(__FILE__, __LINE__);
	ServerResetRecordedPlayerStates();
	recordedPlayerStateQueue.Empty();
	passedTime = 0.f;
	recording = true;
DebugPrint(__FILE__, __LINE__);}

bool UPlayerStateRecorder::ServerResetRecordedPlayerStates_Validate() {DebugPrint(__FILE__, __LINE__);
	return true;
DebugPrint(__FILE__, __LINE__);}

void UPlayerStateRecorder::ServerResetRecordedPlayerStates_Implementation() {DebugPrint(__FILE__, __LINE__);
	RecordedPlayerStates.Empty();
	pressedKeys.Empty();
	releasedKeys.Empty();
DebugPrint(__FILE__, __LINE__);}

bool UPlayerStateRecorder::ServerRecordKeyPressed_Validate(const FString &key) {DebugPrint(__FILE__, __LINE__);
	return true;
DebugPrint(__FILE__, __LINE__);}

void UPlayerStateRecorder::ServerRecordKeyPressed_Implementation(const FString &key) {DebugPrint(__FILE__, __LINE__);
	pressedKeys.Add(key);
DebugPrint(__FILE__, __LINE__);}

bool UPlayerStateRecorder::ServerRecordKeyReleased_Validate(const FString &key) {DebugPrint(__FILE__, __LINE__);
	return true;
DebugPrint(__FILE__, __LINE__);}

void UPlayerStateRecorder::ServerRecordKeyReleased_Implementation(const FString &key) {DebugPrint(__FILE__, __LINE__);
	releasedKeys.Add(key);
DebugPrint(__FILE__, __LINE__);}

void UPlayerStateRecorder::RecordMoveForward(float val) {DebugPrint(__FILE__, __LINE__);
	if(movingForward == val) {DebugPrint(__FILE__, __LINE__);
		return;
	DebugPrint(__FILE__, __LINE__);}

	if(val > 0) {DebugPrint(__FILE__, __LINE__);
		ServerRecordKeyPressed("MoveForward");
	DebugPrint(__FILE__, __LINE__);} else if(val < 0) {DebugPrint(__FILE__, __LINE__);
		ServerRecordKeyPressed("MoveBackward");
	DebugPrint(__FILE__, __LINE__);}

	if(movingForward > 0) {DebugPrint(__FILE__, __LINE__);
		ServerRecordKeyReleased("MoveForward");
	DebugPrint(__FILE__, __LINE__);} else if(movingForward < 0) {DebugPrint(__FILE__, __LINE__);
		ServerRecordKeyReleased("MoveBackward");
	DebugPrint(__FILE__, __LINE__);}

	movingForward = val;
DebugPrint(__FILE__, __LINE__);}

void UPlayerStateRecorder::RecordMoveRight(float val) {DebugPrint(__FILE__, __LINE__);
	if(movingRight == val) {DebugPrint(__FILE__, __LINE__);
		return;
	DebugPrint(__FILE__, __LINE__);}

	if(val > 0) {DebugPrint(__FILE__, __LINE__);
		ServerRecordKeyPressed("MoveRight");
	DebugPrint(__FILE__, __LINE__);} else if(val < 0) {DebugPrint(__FILE__, __LINE__);
		ServerRecordKeyPressed("MoveLeft");
	DebugPrint(__FILE__, __LINE__);}

	if(movingRight > 0) {DebugPrint(__FILE__, __LINE__);
		ServerRecordKeyReleased("MoveRight");
	DebugPrint(__FILE__, __LINE__);} else if(movingRight < 0) {DebugPrint(__FILE__, __LINE__);
		ServerRecordKeyReleased("MoveLeft");
	DebugPrint(__FILE__, __LINE__);}

	movingRight = val;
DebugPrint(__FILE__, __LINE__);}

void UPlayerStateRecorder::RecordKey(FString key, EInputEvent eventType) {DebugPrint(__FILE__, __LINE__);
	if(eventType == IE_Pressed)
		ServerRecordKeyPressed(key);
	if(eventType == IE_Released)
		ServerRecordKeyReleased(key);
DebugPrint(__FILE__, __LINE__);}

template<EInputEvent eventType>
void UPlayerStateRecorder::RecordJump() {DebugPrint(__FILE__, __LINE__);
	RecordKey("Jump", eventType);
DebugPrint(__FILE__, __LINE__);}

template<EInputEvent eventType>
void UPlayerStateRecorder::RecordSprint() {DebugPrint(__FILE__, __LINE__);
	RecordKey("Sprint", eventType);
DebugPrint(__FILE__, __LINE__);}

template<EInputEvent eventType>
void UPlayerStateRecorder::RecordAim() {DebugPrint(__FILE__, __LINE__);
	RecordKey("Aim", eventType);
DebugPrint(__FILE__, __LINE__);}

template<EInputEvent eventType>
void UPlayerStateRecorder::RecordReload() {DebugPrint(__FILE__, __LINE__);
	RecordKey("Reload", eventType);
DebugPrint(__FILE__, __LINE__);}

template<EInputEvent eventType>
void UPlayerStateRecorder::RecordEquipNextWeapon() {DebugPrint(__FILE__, __LINE__);
	RecordKey("EquipNextWeapon", eventType);
DebugPrint(__FILE__, __LINE__);}

template<EInputEvent eventType>
void UPlayerStateRecorder::RecordEquipPreviousWeapon() {DebugPrint(__FILE__, __LINE__);
	RecordKey("EquipPreviousWeapon", eventType);
DebugPrint(__FILE__, __LINE__);}

template<int index, EInputEvent eventType>
void UPlayerStateRecorder::RecordEquipSpecificWeapon() {DebugPrint(__FILE__, __LINE__);
	RecordKey(FString("EquipSpecificWeapon") + FString::FromInt(index), eventType);
DebugPrint(__FILE__, __LINE__);}
