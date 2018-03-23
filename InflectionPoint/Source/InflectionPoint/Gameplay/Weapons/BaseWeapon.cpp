// Fill out your copyright notice in the Description page of Project Settings.

#include "InflectionPoint.h"
#include "Gameplay/Characters/BaseCharacter.h"
#include "Gameplay/Characters/ReplayCharacterBase.h"
#include "BaseWeapon.h"


void ABaseWeapon::GetLifetimeReplicatedProps(TArray< FLifetimeProperty > & OutLifetimeProps) const {DebugPrint(__FILE__, __LINE__);
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(ABaseWeapon, CurrentAmmoInClip);
	DOREPLIFETIME(ABaseWeapon, CurrentAmmo);
	DOREPLIFETIME(ABaseWeapon, OwningCharacter);
DebugPrint(__FILE__, __LINE__);}

// Sets default values
ABaseWeapon::ABaseWeapon() {DebugPrint(__FILE__, __LINE__);
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;

	// Create a gun mesh component
	Mesh1P = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Mesh1P"));
	Mesh1P->SetOnlyOwnerSee(true);			// only the owning player will see this mesh
	Mesh1P->bCastDynamicShadow = false;
	Mesh1P->CastShadow = false;

	RootComponent = Mesh1P;

	Mesh1P->RelativeScale3D = FVector(.4, .4, .4);

	// Create the '3rd person' gun mesh
	Mesh3P = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Mesh3P"));
	Mesh3P->SetOwnerNoSee(true);
	Mesh3P->bCastHiddenShadow = true;

	// MuzzleLocation, where shots will be spawned
	FP_MuzzleLocation = CreateDefaultSubobject<USceneComponent>(TEXT("MuzzleLocation"));
	FP_MuzzleLocation->SetupAttachment(Mesh1P);
	FP_MuzzleLocation->SetRelativeLocation(FVector(0.2f, 60.f, 11.f));

	// MuzzleLocation that will be used instead when the player is aiming
	FP_Aim_MuzzleLocation = CreateDefaultSubobject<USceneComponent>(TEXT("FPAimMuzzleLocation"));
	FP_Aim_MuzzleLocation->SetupAttachment(Mesh1P);
	FP_Aim_MuzzleLocation->SetRelativeLocation(FVector(9.f, 81.f, 11.f));

	// MuzzleLocation for playing visual fx only
	TP_MuzzleLocation = CreateDefaultSubobject<USceneComponent>(TEXT("TPMuzzleLocation"));
	TP_MuzzleLocation->SetupAttachment(Mesh3P);
	TP_MuzzleLocation->SetRelativeLocation(FVector(0.7f, 56.f, 10.8f));

	AnimationNotifyDelegate.BindUFunction(this, "ReloadAnimationNotifyCallback");
	AnimationEndDelegate.BindUFunction(this, "ReloadAnimationEndCallback");
DebugPrint(__FILE__, __LINE__);}

// Called when the game starts or when spawned
void ABaseWeapon::BeginPlay() {DebugPrint(__FILE__, __LINE__);
	Super::BeginPlay();
	CurrentAmmoInClip = CurrentAmmo < 0 ? ClipSize : FMath::Min(CurrentAmmo, ClipSize);

	if(!HasAuthority())
		return; // On Client the Instigator is not set yet

	OwningCharacter = Cast<ABaseCharacter>(Instigator);
	if(!AssertNotNull(OwningCharacter, GetWorld(), __FILE__, __LINE__))
		return;
	Recorder = OwningCharacter->FindComponentByClass<UPlayerStateRecorder>();
	ReattachMuzzleLocation();
DebugPrint(__FILE__, __LINE__);}

void ABaseWeapon::OnRep_Instigator() {DebugPrint(__FILE__, __LINE__);
	OwningCharacter = Cast<ABaseCharacter>(Instigator);
	if(!AssertNotNull(OwningCharacter, GetWorld(), __FILE__, __LINE__))
		return;
	Recorder = OwningCharacter->FindComponentByClass<UPlayerStateRecorder>();
	ReattachMuzzleLocation();
DebugPrint(__FILE__, __LINE__);}

void ABaseWeapon::ReattachMuzzleLocation() {DebugPrint(__FILE__, __LINE__);
	// Reattach MuzzleLocation from weapon to camera
	AttachToOwner();
	FP_MuzzleLocation->AttachToComponent(OwningCharacter->FirstPersonCameraComponent, FAttachmentTransformRules(EAttachmentRule::KeepWorld, true));
	FP_Aim_MuzzleLocation->AttachToComponent(OwningCharacter->FirstPersonCameraComponent, FAttachmentTransformRules(EAttachmentRule::KeepWorld, true));
	if(!equipped) {DebugPrint(__FILE__, __LINE__);
		OnUnequip();
	DebugPrint(__FILE__, __LINE__);}
DebugPrint(__FILE__, __LINE__);}

void ABaseWeapon::DetachFromOwner() {DebugPrint(__FILE__, __LINE__);
	Mesh1P->DetachFromComponent(FDetachmentTransformRules::KeepRelativeTransform);
	Mesh3P->DetachFromComponent(FDetachmentTransformRules::KeepRelativeTransform);
DebugPrint(__FILE__, __LINE__);}

void ABaseWeapon::AttachToOwner() {DebugPrint(__FILE__, __LINE__);
	DetachFromOwner();

	Mesh1P->AttachToComponent(OwningCharacter->Mesh1P, FAttachmentTransformRules(EAttachmentRule::SnapToTarget, true), TEXT("GripPoint"));
	Mesh3P->AttachToComponent(OwningCharacter->Mesh3P, FAttachmentTransformRules(EAttachmentRule::SnapToTarget, true), TEXT("GripPoint"));
DebugPrint(__FILE__, __LINE__);}

// Called every frame
void ABaseWeapon::Tick(float DeltaTime) {DebugPrint(__FILE__, __LINE__);
	Super::Tick(DeltaTime);
	if(!HasAuthority()) {DebugPrint(__FILE__, __LINE__);
		return;
	DebugPrint(__FILE__, __LINE__);}

	passedTime += DeltaTime;

	if(CurrentAmmoInClip == 0 && CurrentAmmo != 0 && CurrentState != EWeaponState::RELOADING) {DebugPrint(__FILE__, __LINE__);
		StartTimer(this, GetWorld(), "Reload", 0.1f + ReloadDelay, false); // use timer to avoid reload animation loops
	DebugPrint(__FILE__, __LINE__);} else if(CurrentState == EWeaponState::FIRING && passedTime - LastShotTimeStamp >= FireInterval) {DebugPrint(__FILE__, __LINE__);
		passedTime = 0;
		Fire();
	DebugPrint(__FILE__, __LINE__);} else if(Recorder && IsReplaySimulatedFirePressed) {DebugPrint(__FILE__, __LINE__);
		IsReplaySimulatedFirePressed = false;
		Recorder->ServerRecordKeyReleased("WeaponFired");
	DebugPrint(__FILE__, __LINE__);}
DebugPrint(__FILE__, __LINE__);}

void ABaseWeapon::StartFire() {DebugPrint(__FILE__, __LINE__);
	if(CurrentState == EWeaponState::IDLE) {DebugPrint(__FILE__, __LINE__);
		CurrentState = EWeaponState::FIRING;
	DebugPrint(__FILE__, __LINE__);}
DebugPrint(__FILE__, __LINE__);}

void ABaseWeapon::Fire() {DebugPrint(__FILE__, __LINE__);
	if(CurrentAmmo == 0 && CurrentAmmoInClip == 0) {DebugPrint(__FILE__, __LINE__);
		SpawnNoAmmoSound();
	DebugPrint(__FILE__, __LINE__);} else {DebugPrint(__FILE__, __LINE__);
		if(CurrentAmmo == 0)
			return;

		if(Recorder) {DebugPrint(__FILE__, __LINE__);
			IsReplaySimulatedFirePressed = true;
			Recorder->ServerRecordKeyPressed("WeaponFired");
		DebugPrint(__FILE__, __LINE__);}
		PreExecuteFire();
		for(int i = 0; i < FireShotNum; i++)
			ExecuteFire();
		PostExecuteFire();
		CurrentAmmoInClip--;
		CurrentAmmo--;
		ForceNetUpdate();
		MulticastFireExecuted();
	DebugPrint(__FILE__, __LINE__);}
	if(!AutoFire)
		CurrentState = EWeaponState::IDLE;
DebugPrint(__FILE__, __LINE__);}

void ABaseWeapon::OnEquip() {DebugPrint(__FILE__, __LINE__);
	if(!AssertNotNull(OwningCharacter, GetWorld(), __FILE__, __LINE__))
		return;
	equipped = true;
	SetActorTickEnabled(true);
	OwningCharacter->Mesh1P->GetAnimInstance()->Montage_Play(EquipAnimation1P);
	OwningCharacter->Mesh3P->GetAnimInstance()->Montage_Play(EquipAnimation3P);

	AttachToOwner();

	Mesh1P->SetHiddenInGame(false);
	Mesh3P->SetHiddenInGame(false);
	Mesh3P->bCastHiddenShadow = true;

	CurrentState = EWeaponState::IDLE;
DebugPrint(__FILE__, __LINE__);}

void ABaseWeapon::OnUnequip() {DebugPrint(__FILE__, __LINE__);
	equipped = false;
	SetActorTickEnabled(false);

	DetachFromOwner();

	Mesh1P->SetHiddenInGame(true);
	Mesh3P->SetHiddenInGame(true);
	Mesh3P->bCastHiddenShadow = false;

	OwningCharacter->Mesh1P->GetAnimInstance()->OnPlayMontageNotifyBegin.Remove(AnimationNotifyDelegate);
	OwningCharacter->Mesh1P->GetAnimInstance()->OnMontageEnded.Remove(AnimationEndDelegate);

	OwningCharacter->Mesh1P->GetAnimInstance()->Montage_Stop(0, ReloadAnimation1P);
DebugPrint(__FILE__, __LINE__);}

void ABaseWeapon::MulticastFireExecuted_Implementation() {DebugPrint(__FILE__, __LINE__);
	SpawnMuzzleFX();
	SpawnFireSound();
	PlayFireAnimation();
DebugPrint(__FILE__, __LINE__);}

void ABaseWeapon::SpawnFireSound() {DebugPrint(__FILE__, __LINE__);
	// try and play the sound if specified
	if(FireSound != NULL) {DebugPrint(__FILE__, __LINE__);
		UGameplayStatics::SpawnSoundAttached(FireSound, OwningCharacter->Mesh1P);
	DebugPrint(__FILE__, __LINE__);}
DebugPrint(__FILE__, __LINE__);}

void ABaseWeapon::SpawnNoAmmoSound() {DebugPrint(__FILE__, __LINE__);
	// try and play the sound if specified
	if(NoAmmoSound != NULL) {DebugPrint(__FILE__, __LINE__);
		UGameplayStatics::SpawnSoundAttached(NoAmmoSound, OwningCharacter->Mesh1P);
	DebugPrint(__FILE__, __LINE__);}
DebugPrint(__FILE__, __LINE__);}

void ABaseWeapon::PlayFireAnimation() {DebugPrint(__FILE__, __LINE__);
	// try and play a firing animation if specified
	if(FireAnimation != NULL) {DebugPrint(__FILE__, __LINE__);
		// Get the animation object for the arms mesh
		UAnimInstance* AnimInstance = OwningCharacter->Mesh1P->GetAnimInstance();
		if(AnimInstance != NULL) {DebugPrint(__FILE__, __LINE__);
			AnimInstance->Montage_Play(FireAnimation, 1.f);
		DebugPrint(__FILE__, __LINE__);}
	DebugPrint(__FILE__, __LINE__);}
	Mesh1P->PlayAnimation(FireAnimationWeapon1P, false);
DebugPrint(__FILE__, __LINE__);}

void ABaseWeapon::StopFire() {DebugPrint(__FILE__, __LINE__);
	if(CurrentState == EWeaponState::FIRING) {DebugPrint(__FILE__, __LINE__);
		CurrentState = EWeaponState::IDLE;
	DebugPrint(__FILE__, __LINE__);}
DebugPrint(__FILE__, __LINE__);}

void ABaseWeapon::Reload() {DebugPrint(__FILE__, __LINE__);
	if(CurrentState != EWeaponState::RELOADING && CurrentAmmoInClip != ClipSize && CurrentAmmoInClip != CurrentAmmo) {DebugPrint(__FILE__, __LINE__);
		OwningCharacter->Mesh1P->GetAnimInstance()->OnPlayMontageNotifyBegin.AddUnique(AnimationNotifyDelegate);
		OwningCharacter->Mesh1P->GetAnimInstance()->OnMontageEnded.AddUnique(AnimationEndDelegate);

		CurrentState = EWeaponState::RELOADING;
		MulticastPlayReloadAnimation();
		OwningCharacter->Mesh1P->GetAnimInstance()->Montage_Play(ReloadAnimation1P);
		OwningCharacter->Mesh3P->GetAnimInstance()->Montage_Play(ReloadAnimation3P);
		Mesh1P->PlayAnimation(ReloadAnimationWeapon1P, false);
	DebugPrint(__FILE__, __LINE__);}
DebugPrint(__FILE__, __LINE__);}

void ABaseWeapon::MulticastPlayReloadAnimation_Implementation() {DebugPrint(__FILE__, __LINE__);
	if(!HasAuthority()) {DebugPrint(__FILE__, __LINE__);
		OwningCharacter->Mesh1P->GetAnimInstance()->Montage_Play(ReloadAnimation1P);
		OwningCharacter->Mesh3P->GetAnimInstance()->Montage_Play(ReloadAnimation3P);
		Mesh1P->PlayAnimation(ReloadAnimationWeapon1P, false);
	DebugPrint(__FILE__, __LINE__);}
DebugPrint(__FILE__, __LINE__);}

void ABaseWeapon::ReloadAnimationNotifyCallback(FName NotifyName, const FBranchingPointNotifyPayload& BranchingPointPayload) {DebugPrint(__FILE__, __LINE__);
	if(NotifyName.ToString() == "RefillAmmo") {DebugPrint(__FILE__, __LINE__);
		CurrentAmmoInClip = CurrentAmmo < 0 ? ClipSize : FMath::Min(CurrentAmmo, ClipSize);
		ForceNetUpdate();
	DebugPrint(__FILE__, __LINE__);} else if(NotifyName.ToString() == "EnableFiring") {DebugPrint(__FILE__, __LINE__);
		CurrentState = EWeaponState::IDLE;
	DebugPrint(__FILE__, __LINE__);}
DebugPrint(__FILE__, __LINE__);}

void ABaseWeapon::ReloadAnimationEndCallback(UAnimMontage* Montage, bool bInterrupted) {DebugPrint(__FILE__, __LINE__);
	if(Montage == ReloadAnimation1P && CurrentState == EWeaponState::RELOADING) {DebugPrint(__FILE__, __LINE__);
		CurrentState = EWeaponState::IDLE;
	DebugPrint(__FILE__, __LINE__);}
DebugPrint(__FILE__, __LINE__);}

void ABaseWeapon::SpawnMuzzleFX() {DebugPrint(__FILE__, __LINE__);
	if(!MuzzleFX)
		return;

	UParticleSystemComponent* mesh1pFX = UGameplayStatics::SpawnEmitterAttached(MuzzleFX, Mesh1P, NAME_None);
	if(mesh1pFX) {DebugPrint(__FILE__, __LINE__);
		mesh1pFX->SetRelativeScale3D(MuzzleFXScale);
		mesh1pFX->SetWorldLocation(GetFPMuzzleLocation());
		mesh1pFX->SetWorldRotation(GetAimDirection());
		mesh1pFX->bOwnerNoSee = false;
		mesh1pFX->bOnlyOwnerSee = true;
		if(MuzzleFXDuration > 0)
			StartTimer(this, GetWorld(), "DecativateParticleSystem", MuzzleFXDuration, false, mesh1pFX);
	DebugPrint(__FILE__, __LINE__);}
	UParticleSystemComponent* mesh3pFX = UGameplayStatics::SpawnEmitterAttached(MuzzleFX, Mesh3P, NAME_None);
	if(mesh3pFX) {DebugPrint(__FILE__, __LINE__);
		mesh3pFX->SetWorldLocation(GetTPMuzzleLocation());
		mesh3pFX->SetWorldRotation(GetAimDirection());
		mesh3pFX->bOwnerNoSee = true;
		mesh3pFX->bOnlyOwnerSee = false;
		if(MuzzleFXDuration > 0)
			StartTimer(this, GetWorld(), "DecativateParticleSystem", MuzzleFXDuration, false, mesh3pFX);
	DebugPrint(__FILE__, __LINE__);}
DebugPrint(__FILE__, __LINE__);}

void ABaseWeapon::DecativateParticleSystem(UParticleSystemComponent* effect) {DebugPrint(__FILE__, __LINE__);
	effect->DeactivateSystem();
DebugPrint(__FILE__, __LINE__);}

FRotator ABaseWeapon::GetAimDirection() {DebugPrint(__FILE__, __LINE__);
	return OwningCharacter->FirstPersonCameraComponent->GetComponentRotation();
DebugPrint(__FILE__, __LINE__);}

FVector ABaseWeapon::GetFPMuzzleLocation() {DebugPrint(__FILE__, __LINE__);
	if(OwningCharacter && OwningCharacter->IsAiming) {DebugPrint(__FILE__, __LINE__);
		return FP_Aim_MuzzleLocation->GetComponentLocation();
	DebugPrint(__FILE__, __LINE__);}
	return FP_MuzzleLocation->GetComponentLocation();
DebugPrint(__FILE__, __LINE__);}

FVector ABaseWeapon::GetTPMuzzleLocation() {DebugPrint(__FILE__, __LINE__);
	return TP_MuzzleLocation->GetComponentLocation();
DebugPrint(__FILE__, __LINE__);}

void ABaseWeapon::PreExecuteFire() {DebugPrint(__FILE__, __LINE__);DebugPrint(__FILE__, __LINE__);}
void ABaseWeapon::PostExecuteFire() {DebugPrint(__FILE__, __LINE__);DebugPrint(__FILE__, __LINE__);}
