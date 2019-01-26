// Fill out your copyright notice in the Description page of Project Settings.

#include "InflectionPoint.h"
#include "ReplayCharacterBase.h"
#include "Gameplay/Controllers/AIControllerBase.h"
#include "Gamemodes/TDMGameStateBase.h"
#include "Utils/TimerFunctions.h"


void AReplayCharacterBase::GetLifetimeReplicatedProps(TArray< FLifetimeProperty > & OutLifetimeProps) const {DebugPrint(__FILE__, __LINE__);
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AReplayCharacterBase, ReplayIndex);
DebugPrint(__FILE__, __LINE__);}

// Sets default values for this component's properties
AReplayCharacterBase::AReplayCharacterBase() {DebugPrint(__FILE__, __LINE__);
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryActorTick.bCanEverTick = true;

	// Create another Mesh3P for post process effects
	Mesh3PPostProcess = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Mesh3PPostProcess"));
	Mesh3PPostProcess->SetupAttachment(GetCapsuleComponent());
	Mesh3PPostProcess->SetOwnerNoSee(true);
	Mesh3PPostProcess->RelativeLocation = Mesh3P->RelativeLocation;
	Mesh3PPostProcess->RelativeRotation = Mesh3P->RelativeRotation;
DebugPrint(__FILE__, __LINE__);}

void AReplayCharacterBase::BeginPlay() {DebugPrint(__FILE__, __LINE__);
	Super::BeginPlay();
	//PrimaryActorTick.bCanEverTick = true;
	Mesh3PPostProcess->SetMasterPoseComponent(Mesh3P);
DebugPrint(__FILE__, __LINE__);}

bool AReplayCharacterBase::IsReadyForInitialization() {DebugPrint(__FILE__, __LINE__);
	if(!GetController()) {DebugPrint(__FILE__, __LINE__);
		return false;
	DebugPrint(__FILE__, __LINE__);}
	APlayerController* owningController = Cast<AAIControllerBase>(GetController())->OwningPlayerController;
	if(!owningController) {DebugPrint(__FILE__, __LINE__);
		return false;
	DebugPrint(__FILE__, __LINE__);}
	if(!owningController->PlayerState) {DebugPrint(__FILE__, __LINE__);
		return false;
	DebugPrint(__FILE__, __LINE__);}
	return true;
DebugPrint(__FILE__, __LINE__);}

void AReplayCharacterBase::Initialize() {DebugPrint(__FILE__, __LINE__);
	Super::Initialize();
	MulticastUpdateCustomDepthStencil();
DebugPrint(__FILE__, __LINE__);}

void AReplayCharacterBase::StartReplay() {DebugPrint(__FILE__, __LINE__);
	replayIndex = 0;
	isReplaying = true;
DebugPrint(__FILE__, __LINE__);}

void AReplayCharacterBase::MulticastUpdateCustomDepthStencil_Implementation() {DebugPrint(__FILE__, __LINE__);
	auto controller = GetWorld()->GetFirstPlayerController();
	if(controller) {DebugPrint(__FILE__, __LINE__);
		auto playerState = Cast<ATDMPlayerStateBase>(controller->PlayerState);
		bool isInSameTeam = playerState ? playerState->Team == CharacterInfoProvider->GetCharacterInfo().Team : false;
		Mesh3PPostProcess->SetCustomDepthStencilValue(isInSameTeam ? 11 : 21);
	DebugPrint(__FILE__, __LINE__);}
DebugPrint(__FILE__, __LINE__);}

void AReplayCharacterBase::SetReplayData(TArray<FRecordedPlayerState> RecordData) {DebugPrint(__FILE__, __LINE__);
	recordData = RecordData;
DebugPrint(__FILE__, __LINE__);}

void AReplayCharacterBase::StopReplay() {DebugPrint(__FILE__, __LINE__);
	isReplaying = false;
	replayIndex = 0;
DebugPrint(__FILE__, __LINE__);}

void AReplayCharacterBase::TransformToInflectionPoint() {DebugPrint(__FILE__, __LINE__);
	if(!MortalityProvider->IsAlive()) {DebugPrint(__FILE__, __LINE__);
		return;
	DebugPrint(__FILE__, __LINE__);}

	MortalityProvider->Invincible = true;
	SetLifeSpan(2);

	FActorSpawnParameters ActorSpawnParams;
	ActorSpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	ActorSpawnParams.Instigator = this;
	GetWorld()->SpawnActor<AActor>(InflectionPoint, GetTransform().GetLocation(), FRotator(GetTransform().GetRotation()), ActorSpawnParams);

	MulticastShowDematerializeAnimation();
DebugPrint(__FILE__, __LINE__);}

void AReplayCharacterBase::MulticastShowDematerializeAnimation_Implementation() {DebugPrint(__FILE__, __LINE__);
	dematerializeInstanceDynamic = UMaterialInstanceDynamic::Create(DematerializeMaterial, Mesh3P);
	ATDMGameStateBase* gameState = Cast<ATDMGameStateBase>(UGameplayStatics::GetGameState(GetWorld()));
	dematerializeInstanceDynamic->SetVectorParameterValue("BaseColor", gameState->GetTeamColor(CharacterInfoProvider->GetCharacterInfo().Team));

	OverrideMaterials(Mesh3P, dematerializeInstanceDynamic);
	OverrideMaterials(Mesh3PPostProcess, dematerializeInstanceDynamic);
	if(CurrentWeapon)
		OverrideMaterials(CurrentWeapon->Mesh3P, dematerializeInstanceDynamic);

	FOnTimelineFloat callback{};
	callback.BindUFunction(this, FName{TEXT("DematerializeCallback")});
	MaterializeTimeline->AddInterpFloat(DematerializeCurve, callback, FName{TEXT("DematerializeTimelineAnimation")});

	MaterializeTimeline->Play();
DebugPrint(__FILE__, __LINE__);}

void AReplayCharacterBase::OverrideMaterials(USkeletalMeshComponent* MeshComponent, UMaterialInterface* Material) {DebugPrint(__FILE__, __LINE__);
	for(int i = 0; i < MeshComponent->GetNumMaterials(); i++) {DebugPrint(__FILE__, __LINE__);
		if(MeshComponent->GetMaterial(i)->GetFullName() != MaterialToIgnore->GetFullName()) {DebugPrint(__FILE__, __LINE__);
			MeshComponent->SetMaterial(i, Material);
		DebugPrint(__FILE__, __LINE__);}
	DebugPrint(__FILE__, __LINE__);}
DebugPrint(__FILE__, __LINE__);}

void AReplayCharacterBase::DematerializeCallback(float value) {DebugPrint(__FILE__, __LINE__);
	dematerializeInstanceDynamic->SetScalarParameterValue("Opacity", value);
DebugPrint(__FILE__, __LINE__);}

void AReplayCharacterBase::Tick(float deltaTime) {DebugPrint(__FILE__, __LINE__);
	Super::Tick(deltaTime);
	if(!isReplaying)
		return;
	passedTime += deltaTime;
	passedTimeSinceLastCorrection += deltaTime;

	// Correct position 
	bool shouldCorrectPosition = CurrentPositionShouldBeCorrected();
	if(shouldCorrectPosition)
		CorrectPosition(recordData[replayIndex].Position);

	// Draw debug sphere
	if(CreateDebugCorrectionSpheres)
		DrawDebugSphereAtCurrentPosition(shouldCorrectPosition);

	UpdateKeys();

	// Call Hold for all currently pressed buttons
	for(auto &key : pressedKeys) {DebugPrint(__FILE__, __LINE__);
		HoldKey(key);
	DebugPrint(__FILE__, __LINE__);}

	// stop replay when end of recordData reached
	if(HasFinishedReplaying() && isReplaying) {DebugPrint(__FILE__, __LINE__);
		OnFinishedReplaying.Broadcast();
		StopReplay();
		StartTimer(this, GetWorld(), "TransformToInflectionPoint", 1.f, false);
	DebugPrint(__FILE__, __LINE__);}
DebugPrint(__FILE__, __LINE__);}

bool AReplayCharacterBase::HasFinishedReplaying() {DebugPrint(__FILE__, __LINE__);
	return replayIndex >= recordData.Num();
DebugPrint(__FILE__, __LINE__);}

void AReplayCharacterBase::UpdateKeys() {DebugPrint(__FILE__, __LINE__);
	// iterate through all record data since last tick until now
	for(; !HasFinishedReplaying() && recordData[replayIndex].Timestamp <= passedTime; replayIndex++) {DebugPrint(__FILE__, __LINE__);
		UpdateRotation();
		auto recordDataStep = recordData[replayIndex];
		UpdatePressedKeys(recordDataStep);
		UpdateReleasedKeys(recordDataStep);
	DebugPrint(__FILE__, __LINE__);}
DebugPrint(__FILE__, __LINE__);}

void AReplayCharacterBase::UpdateRotation() {DebugPrint(__FILE__, __LINE__);
	if(replayIndex == 0) {DebugPrint(__FILE__, __LINE__);
		return;
	DebugPrint(__FILE__, __LINE__);}
	// Update Rotation
	ApplyYaw(recordData[replayIndex].CapsuleYaw);
	ApplyPitch(recordData[replayIndex].CameraPitch);
DebugPrint(__FILE__, __LINE__);}

void AReplayCharacterBase::UpdatePressedKeys(FRecordedPlayerState &recordDataStep) {DebugPrint(__FILE__, __LINE__);
	for(auto &item : recordDataStep.PressedKeys) {DebugPrint(__FILE__, __LINE__);
		PressKey(item);
		pressedKeys.Add(item);
	DebugPrint(__FILE__, __LINE__);}
DebugPrint(__FILE__, __LINE__);}

void AReplayCharacterBase::UpdateReleasedKeys(FRecordedPlayerState &recordDataStep) {DebugPrint(__FILE__, __LINE__);
	for(auto &item : recordDataStep.ReleasedKeys) {DebugPrint(__FILE__, __LINE__);
		ReleaseKey(item);
		pressedKeys.Remove(item);
	DebugPrint(__FILE__, __LINE__);}
DebugPrint(__FILE__, __LINE__);}

void AReplayCharacterBase::PressKey(FString key) {DebugPrint(__FILE__, __LINE__);
	if(key == "Jump") {DebugPrint(__FILE__, __LINE__);
		Jump();
	DebugPrint(__FILE__, __LINE__);} else if(key == "Aim") {DebugPrint(__FILE__, __LINE__);
		StartAiming();
	DebugPrint(__FILE__, __LINE__);} else if(key == "Sprint") {DebugPrint(__FILE__, __LINE__);
		EnableSprint();
	DebugPrint(__FILE__, __LINE__);} else if(key == "WeaponFired") {DebugPrint(__FILE__, __LINE__);
		CurrentWeapon->Fire();
	DebugPrint(__FILE__, __LINE__);} else if(key == "Reload") {DebugPrint(__FILE__, __LINE__);
		CurrentWeapon->Reload();
	DebugPrint(__FILE__, __LINE__);} else if(key == "EquipNextWeapon") {DebugPrint(__FILE__, __LINE__);
		ServerEquipNextWeapon();
	DebugPrint(__FILE__, __LINE__);} else if(key == "EquipPreviousWeapon") {DebugPrint(__FILE__, __LINE__);
		ServerEquipPreviousWeapon();
	DebugPrint(__FILE__, __LINE__);} else if(key.Contains("EquipSpecificWeapon")) {DebugPrint(__FILE__, __LINE__);
		auto str = FString(key); // to not alter string
		str.RemoveFromStart("EquipSpecificWeapon");
		int index = FCString::Atoi(*str);
		ServerEquipSpecificWeapon((EInventorySlotPosition)index);
	DebugPrint(__FILE__, __LINE__);}
DebugPrint(__FILE__, __LINE__);}

void AReplayCharacterBase::HoldKey(FString key) {DebugPrint(__FILE__, __LINE__);
	if(key == "MoveForward") {DebugPrint(__FILE__, __LINE__);
		MoveForward(1);
	DebugPrint(__FILE__, __LINE__);} else if(key == "MoveBackward") {DebugPrint(__FILE__, __LINE__);
		MoveForward(-1);
	DebugPrint(__FILE__, __LINE__);} else if(key == "MoveLeft") {DebugPrint(__FILE__, __LINE__);
		MoveRight(-1);
	DebugPrint(__FILE__, __LINE__);} else if(key == "MoveRight") {DebugPrint(__FILE__, __LINE__);
		MoveRight(1);
	DebugPrint(__FILE__, __LINE__);}
DebugPrint(__FILE__, __LINE__);}

void AReplayCharacterBase::ReleaseKey(FString key) {DebugPrint(__FILE__, __LINE__);
	if(key == "Sprint") {DebugPrint(__FILE__, __LINE__);
		DisableSprint();
	DebugPrint(__FILE__, __LINE__);} else if(key == "Aim") {DebugPrint(__FILE__, __LINE__);
		StopAiming();
	DebugPrint(__FILE__, __LINE__);}
DebugPrint(__FILE__, __LINE__);}

void AReplayCharacterBase::ApplyYaw(float value) {DebugPrint(__FILE__, __LINE__);
	FRotator rot = GetCapsuleComponent()->GetComponentRotation();
	rot.Yaw = value;
	rot.Roll = 0;
	rot.Pitch = 0;
	GetCapsuleComponent()->SetWorldRotation(rot);

	FRotator rot2 = FirstPersonCameraComponent->GetComponentRotation();
	rot2.Yaw = value;
	FirstPersonCameraComponent->SetWorldRotation(rot2);
DebugPrint(__FILE__, __LINE__);}

void AReplayCharacterBase::ApplyPitch(float value) {DebugPrint(__FILE__, __LINE__);
	FRotator rot = FirstPersonCameraComponent->GetComponentRotation();
	rot.Pitch = value;
	rot.Roll = 0;
	FirstPersonCameraComponent->SetWorldRotation(rot);
DebugPrint(__FILE__, __LINE__);}

bool AReplayCharacterBase::CurrentPositionShouldBeCorrected() {DebugPrint(__FILE__, __LINE__);
	if(replayIndex == 0)
		return false;

	return CorrectPositions;
DebugPrint(__FILE__, __LINE__);}

bool AReplayCharacterBase::CurrentPositionIsInCorrectionRadius(float radius) {DebugPrint(__FILE__, __LINE__);
	if(radius < 0)
		return true;

	FVector actualPosition = GetTransform().GetLocation();
	FVector correctPosition = recordData[replayIndex - 1].Position;

	return FVector::Dist(actualPosition, correctPosition) <= radius;
DebugPrint(__FILE__, __LINE__);}

void AReplayCharacterBase::DrawDebugSphereAtCurrentPosition(bool positionHasBeenCorrected) {DebugPrint(__FILE__, __LINE__);
	FColor sphereColor = positionHasBeenCorrected ? DebugHitColor : DebugMissColor;
	DrawDebugSphere(GetWorld(), GetTransform().GetLocation(), CorrectionRadius, 8, sphereColor, true);
DebugPrint(__FILE__, __LINE__);}

void AReplayCharacterBase::CorrectPosition(FVector correctPosition) {DebugPrint(__FILE__, __LINE__);
	SetActorLocation(correctPosition);
	passedTimeSinceLastCorrection = 0;
DebugPrint(__FILE__, __LINE__);}