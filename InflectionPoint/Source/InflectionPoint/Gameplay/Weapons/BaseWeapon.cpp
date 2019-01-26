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
	DOREPLIFETIME(ABaseWeapon, CurrentState);
	DOREPLIFETIME(ABaseWeapon, shouldPlayFireFX);
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

	AISuitabilityWeaponRangeCurve.GetRichCurve()->DefaultValue = 1.0;
	AISuitabilityWeaponRangeCurve.GetRichCurve()->AddKey(0, 1.0);
DebugPrint(__FILE__, __LINE__);}

void ABaseWeapon::BeginPlay() {DebugPrint(__FILE__, __LINE__);
	Super::BeginPlay();
	CurrentAmmoInClip = CurrentAmmo < 0 ? ClipSize : FMath::Min(CurrentAmmo, ClipSize);

	if(!HasAuthority())
		return; // On Client the Instigator is not set yet
	if(GetWorld()->WorldType == EWorldType::PIE)
		CurrentAmmo = -1;
	SetupReferences();
DebugPrint(__FILE__, __LINE__);}

void ABaseWeapon::EndPlay(const EEndPlayReason::Type EndPlayReason) {DebugPrint(__FILE__, __LINE__);
	if(FireLoopSoundComponent)
		FireLoopSoundComponent->DestroyComponent();
	if(ChargeSoundComponent)
		ChargeSoundComponent->DestroyComponent();
	StopFire();
	Super::EndPlay(EndPlayReason);
DebugPrint(__FILE__, __LINE__);}

void ABaseWeapon::OnRep_Instigator() {DebugPrint(__FILE__, __LINE__);
	SetupReferences();
DebugPrint(__FILE__, __LINE__);}

void ABaseWeapon::SetupReferences() {DebugPrint(__FILE__, __LINE__);
	OwningCharacter = Cast<ABaseCharacter>(Instigator);
	AssertNotNull(OwningCharacter, GetWorld(), __FILE__, __LINE__);
	Recorder = OwningCharacter->FindComponentByClass<UPlayerStateRecorder>();
	//ReattachMuzzleLocation(); // doesnt work because the muzzle location would end up at the wrong location
	StartTimer(this, GetWorld(), "ReattachMuzzleLocation", 0.7f, false);
DebugPrint(__FILE__, __LINE__);}

bool ABaseWeapon::IsReadyForInitialization() {DebugPrint(__FILE__, __LINE__);
	return OwningCharacter != nullptr;
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
	StopFire();
DebugPrint(__FILE__, __LINE__);}

void ABaseWeapon::AttachToOwner() {DebugPrint(__FILE__, __LINE__);
	DetachFromOwner();
	if(!OwningCharacter) {DebugPrint(__FILE__, __LINE__);
		return;
	DebugPrint(__FILE__, __LINE__);}
	Mesh1P->AttachToComponent(OwningCharacter->Mesh1P, FAttachmentTransformRules(EAttachmentRule::SnapToTarget, true), TEXT("GripPoint"));
	Mesh3P->AttachToComponent(OwningCharacter->Mesh3P, FAttachmentTransformRules(EAttachmentRule::SnapToTarget, true), TEXT("GripPoint"));
DebugPrint(__FILE__, __LINE__);}

void ABaseWeapon::Tick(float DeltaTime) {DebugPrint(__FILE__, __LINE__);
	Super::Tick(DeltaTime);
	if(HasAuthority()) {DebugPrint(__FILE__, __LINE__);
		timeSinceLastShot += DeltaTime;
		timeSinceStartFire += DeltaTime;

		if(CurrentAmmoInClip == 0 && CurrentAmmo != 0 && CurrentState != EWeaponState::RELOADING
			&& CurrentState != EWeaponState::EQUIPPING && timeSinceLastShot >= ReloadDelay) {DebugPrint(__FILE__, __LINE__);
			StartTimer(this, GetWorld(), "Reload", 0.1f, false); // use timer to avoid reload animation loops
		DebugPrint(__FILE__, __LINE__);} else if(CurrentState == EWeaponState::CHARGING && timeSinceStartFire >= ChargeDuration) {DebugPrint(__FILE__, __LINE__);
			ChangeWeaponState(EWeaponState::FIRING);
		DebugPrint(__FILE__, __LINE__);} else if(CurrentState == EWeaponState::FIRING && timeSinceLastShot >= FireInterval) {DebugPrint(__FILE__, __LINE__);
			Fire();
		DebugPrint(__FILE__, __LINE__);} else if(Recorder && RecordKeyReleaseNextTick) {DebugPrint(__FILE__, __LINE__);
			RecordKeyReleaseNextTick = false;
			Recorder->ServerRecordKeyReleased("WeaponFired");
		DebugPrint(__FILE__, __LINE__);}
		// You can not only take the CurrentState because of replays only calling FireOnce()
		shouldPlayFireFX = shouldPlayFireFX && timeSinceLastShot <= FireInterval + 0.1;
	DebugPrint(__FILE__, __LINE__);}
	TogglePersistentSoundFX(FireLoopSoundComponent, FireLoopSound, shouldPlayFireFX);
	TogglePersistentSoundFX(ChargeSoundComponent, ChargeSound, CurrentState == EWeaponState::CHARGING);
DebugPrint(__FILE__, __LINE__);}

void ABaseWeapon::StartFire() {DebugPrint(__FILE__, __LINE__);
	wantsToFire = true;
	timeSinceStartFire = 0;
	if(CurrentAmmo == 0 && CurrentAmmoInClip == 0) {DebugPrint(__FILE__, __LINE__);
		MulticastSpawnNoAmmoSound();
	DebugPrint(__FILE__, __LINE__);} else if(CurrentState == EWeaponState::IDLE && CurrentAmmoInClip > 0) {DebugPrint(__FILE__, __LINE__);
		ChangeWeaponState(EWeaponState::CHARGING);
	DebugPrint(__FILE__, __LINE__);}
DebugPrint(__FILE__, __LINE__);}

void ABaseWeapon::FireOnce() {DebugPrint(__FILE__, __LINE__);
	if(CurrentAmmo == 0 && CurrentAmmoInClip == 0) {DebugPrint(__FILE__, __LINE__);
		MulticastSpawnNoAmmoSound();
	DebugPrint(__FILE__, __LINE__);} else if(CurrentState == EWeaponState::IDLE && CurrentAmmoInClip > 0 && timeSinceLastShot >= FireInterval) {DebugPrint(__FILE__, __LINE__);
		ChangeWeaponState(EWeaponState::FIRING); // No charging for replays
		Fire();
		ChangeWeaponState(EWeaponState::IDLE);
	DebugPrint(__FILE__, __LINE__);}
DebugPrint(__FILE__, __LINE__);}

bool ABaseWeapon::CanFire() {DebugPrint(__FILE__, __LINE__);
	return true;
DebugPrint(__FILE__, __LINE__);}

void ABaseWeapon::Fire() {DebugPrint(__FILE__, __LINE__);
	if(CanFire()) {DebugPrint(__FILE__, __LINE__);
		if(Recorder) {DebugPrint(__FILE__, __LINE__);
			RecordKeyReleaseNextTick = true;
			Recorder->ServerRecordKeyPressed("WeaponFired");
		DebugPrint(__FILE__, __LINE__);}
		if(CurrentAmmoInClip <= 0)
			return;
		shouldPlayFireFX = true;
		timeSinceLastShot = 0;
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
		ChangeWeaponState(EWeaponState::IDLE);
DebugPrint(__FILE__, __LINE__);}

void ABaseWeapon::OnEquip() {DebugPrint(__FILE__, __LINE__);
	timeSinceLastShot = FireInterval; // so you can fire after EquipDelay
	ChangeWeaponState(EWeaponState::EQUIPPING);

	UpdateEquippedState(true);

	StartTimer(this, GetWorld(), "ChangeWeaponState", EquipDelay + 0.001f, false, EWeaponState::IDLE);
DebugPrint(__FILE__, __LINE__);}

void ABaseWeapon::OnUnequip() {DebugPrint(__FILE__, __LINE__);
	wantsToFire = false;
	UpdateEquippedState(false);
DebugPrint(__FILE__, __LINE__);}

void ABaseWeapon::UpdateEquippedState(bool newEquipped) {DebugPrint(__FILE__, __LINE__);
	this->equipped = newEquipped;
	SetActorTickEnabled(newEquipped);
	Mesh1P->SetHiddenInGame(!newEquipped, true);
	Mesh3P->SetHiddenInGame(!newEquipped, true);
	Mesh3P->bCastHiddenShadow = newEquipped;
	newEquipped ? AttachToOwner() : DetachFromOwner();
DebugPrint(__FILE__, __LINE__);}

void ABaseWeapon::MulticastFireExecuted_Implementation() {DebugPrint(__FILE__, __LINE__);
	UGameplayStatics::PlayWorldCameraShake(GetWorld(), FireCameraShake, OwningCharacter->GetActorLocation(), 50, 60);
	SpawnMuzzleFX();
	SpawnFireSound();
	PlayFireAnimation();
	OnFireExecuted.Broadcast();
DebugPrint(__FILE__, __LINE__);}

void ABaseWeapon::SpawnFireSound() {DebugPrint(__FILE__, __LINE__);
	UGameplayStatics::SpawnSoundAttached(FireSound, Mesh1P);
DebugPrint(__FILE__, __LINE__);}

void ABaseWeapon::MulticastSpawnNoAmmoSound_Implementation() {DebugPrint(__FILE__, __LINE__);
	UGameplayStatics::SpawnSoundAttached(NoAmmoSound, Mesh1P);
DebugPrint(__FILE__, __LINE__);}

void ABaseWeapon::TogglePersistentSoundFX(UAudioComponent*& component, class USoundBase* soundClass, bool shouldPlay, float fadeOut) {DebugPrint(__FILE__, __LINE__);
	if(shouldPlay) {DebugPrint(__FILE__, __LINE__);
		if(!component) {DebugPrint(__FILE__, __LINE__);
			component = UGameplayStatics::SpawnSoundAttached(soundClass, Mesh1P, NAME_None, FVector(ForceInit), FRotator::ZeroRotator, EAttachLocation::KeepRelativeOffset, false, 1.f, 1.f, 0.f, nullptr, nullptr, false); // bAutoDestroy=false
		DebugPrint(__FILE__, __LINE__);} else if(!component->IsPlaying()) {DebugPrint(__FILE__, __LINE__);
			component->Play(0);
		DebugPrint(__FILE__, __LINE__);}
	DebugPrint(__FILE__, __LINE__);} else if(component && component->IsPlaying()) {DebugPrint(__FILE__, __LINE__);
		component->FadeOut(fadeOut, 0);
	DebugPrint(__FILE__, __LINE__);}
DebugPrint(__FILE__, __LINE__);}

void ABaseWeapon::PlayFireAnimation() {DebugPrint(__FILE__, __LINE__);
	UAnimInstance* AnimInstance = OwningCharacter->Mesh1P->GetAnimInstance();
	if(AnimInstance != NULL) {DebugPrint(__FILE__, __LINE__);
		AnimInstance->Montage_Play(FireAnimation, 1.f);
	DebugPrint(__FILE__, __LINE__);}
	Mesh1P->PlayAnimation(FireAnimationWeapon1P, false);
DebugPrint(__FILE__, __LINE__);}

void ABaseWeapon::StopFire() {DebugPrint(__FILE__, __LINE__);
	wantsToFire = false;
	shouldPlayFireFX = false;
	TogglePersistentSoundFX(FireLoopSoundComponent, FireLoopSound, false);
	TogglePersistentSoundFX(ChargeSoundComponent, ChargeSound, false);
	if(CurrentState == EWeaponState::FIRING || CurrentState == EWeaponState::CHARGING) {DebugPrint(__FILE__, __LINE__);
		ChangeWeaponState(EWeaponState::IDLE);
	DebugPrint(__FILE__, __LINE__);}
DebugPrint(__FILE__, __LINE__);}

void ABaseWeapon::Reload() {DebugPrint(__FILE__, __LINE__);
	if(CurrentState != EWeaponState::RELOADING && CurrentState != EWeaponState::EQUIPPING && CurrentAmmoInClip != ClipSize && CurrentAmmoInClip != CurrentAmmo) {DebugPrint(__FILE__, __LINE__);
		OwningCharacter->Mesh1P->GetAnimInstance()->OnPlayMontageNotifyBegin.AddUnique(AnimationNotifyDelegate);

		ChangeWeaponState(EWeaponState::RELOADING);
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
		if(wantsToFire) {DebugPrint(__FILE__, __LINE__);
			timeSinceStartFire = 0;
			ChangeWeaponState(EWeaponState::CHARGING);
		DebugPrint(__FILE__, __LINE__);} else {DebugPrint(__FILE__, __LINE__);
			ChangeWeaponState(EWeaponState::IDLE);
		DebugPrint(__FILE__, __LINE__);}
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
	if(effect)
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

void ABaseWeapon::StartAiming() {DebugPrint(__FILE__, __LINE__);
	if(HideWeaponWhenAiming)
		Mesh1P->SetVisibility(false, true);
DebugPrint(__FILE__, __LINE__);}

void ABaseWeapon::StopAiming() {DebugPrint(__FILE__, __LINE__);
	if(HideWeaponWhenAiming)
		Mesh1P->SetVisibility(true, true);
DebugPrint(__FILE__, __LINE__);}

void ABaseWeapon::ChangeWeaponState(EWeaponState newState) {DebugPrint(__FILE__, __LINE__);
	CurrentState = newState;
	MulticastStateChanged(newState);
DebugPrint(__FILE__, __LINE__);}

void ABaseWeapon::MulticastStateChanged_Implementation(EWeaponState newState) {DebugPrint(__FILE__, __LINE__);
	OnStateChanged(newState);
DebugPrint(__FILE__, __LINE__);}

EWeaponState ABaseWeapon::GetCurrentWeaponState() {DebugPrint(__FILE__, __LINE__);
	return CurrentState;
DebugPrint(__FILE__, __LINE__);}

float ABaseWeapon::GetAIWeaponSuitability(ABaseCharacter* shooter, AActor* victim) {DebugPrint(__FILE__, __LINE__);
	if(CurrentAmmo == 0)
		return 0;
	float distance = 1000;
	if(shooter && victim)
		distance = (shooter->GetActorLocation() - victim->GetActorLocation()).Size();
	return AISuitabilityWeaponRangeCurve.GetRichCurveConst()->Eval(distance, 0);
	//return 1.0;
DebugPrint(__FILE__, __LINE__);}

bool ABaseWeapon::ServerIncreaseCurrentAmmo_Validate(int amount) {DebugPrint(__FILE__, __LINE__);
	return true;
DebugPrint(__FILE__, __LINE__);}

void ABaseWeapon::ServerIncreaseCurrentAmmo_Implementation(int amount) {DebugPrint(__FILE__, __LINE__);
	if(MaxAmmo >= 0 && CurrentAmmo + amount > MaxAmmo) {DebugPrint(__FILE__, __LINE__);
		CurrentAmmo = MaxAmmo;
		return;
	DebugPrint(__FILE__, __LINE__);}
	if(CurrentAmmo <= -1)
		return;
	CurrentAmmo += amount;
DebugPrint(__FILE__, __LINE__);}

void ABaseWeapon::PreExecuteFire() {DebugPrint(__FILE__, __LINE__);DebugPrint(__FILE__, __LINE__);}
void ABaseWeapon::PostExecuteFire() {DebugPrint(__FILE__, __LINE__);DebugPrint(__FILE__, __LINE__);}
