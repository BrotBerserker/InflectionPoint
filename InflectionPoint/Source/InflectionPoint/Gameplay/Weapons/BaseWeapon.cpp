// Fill out your copyright notice in the Description page of Project Settings.

#include "InflectionPoint.h"
#include "Gameplay/Characters/BaseCharacter.h"
#include "Gameplay/Characters/ReplayCharacterBase.h"
#include "Engine/ActorChannel.h"
#include "BaseWeapon.h"


void ABaseWeapon::GetLifetimeReplicatedProps(TArray< FLifetimeProperty > & OutLifetimeProps) const {
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(ABaseWeapon, CurrentAmmoInClip);
	DOREPLIFETIME(ABaseWeapon, CurrentAmmo);
	DOREPLIFETIME(ABaseWeapon, OwningCharacter);
	DOREPLIFETIME(ABaseWeapon, CurrentState);
	DOREPLIFETIME(ABaseWeapon, WeaponModi);
	DOREPLIFETIME(ABaseWeapon, CurrentWeaponModusIndex);
}

bool ABaseWeapon::ReplicateSubobjects(class UActorChannel *channel, class FOutBunch *bunch, FReplicationFlags *repFlags) {
	bool wroteSomething = Super::ReplicateSubobjects(channel, bunch, repFlags);

	for(FBaseWeaponModus& mode : WeaponModi) {
		if(mode.SecondaryModule && channel->ReplicateSubobject(mode.SecondaryModule, *bunch, *repFlags))
			wroteSomething = true;
		if(mode.PrimaryModule && channel->ReplicateSubobject(mode.PrimaryModule, *bunch, *repFlags))
			wroteSomething = true;
	}
	return wroteSomething;
}

// Sets default values
ABaseWeapon::ABaseWeapon() {
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
}

void ABaseWeapon::BeginPlay() {
	Super::BeginPlay();
	CurrentAmmoInClip = CurrentAmmo < 0 ? ClipSize : FMath::Min(CurrentAmmo, ClipSize);

	if(!HasAuthority())
		return; // On Client the Instigator is not set yet
	if(GetWorld()->WorldType == EWorldType::PIE)
		CurrentAmmo = -1;
	Setup();
}

void ABaseWeapon::EndPlay(const EEndPlayReason::Type EndPlayReason) {
	GetCurrentWeaponModus().PrimaryModule->Dispose();
	GetCurrentWeaponModus().SecondaryModule->Dispose();
	Super::EndPlay(EndPlayReason);
}

void ABaseWeapon::OnRep_Instigator() {
	Setup(); // Setup the client
}

void ABaseWeapon::Setup() {
	SetupReferences();
	if(HasAuthority())
		SetupWeaponModi();
}

void ABaseWeapon::SetupReferences() {
	OwningCharacter = Cast<ABaseCharacter>(Instigator);
	AssertNotNull(OwningCharacter, GetWorld(), __FILE__, __LINE__);
	Recorder = OwningCharacter->FindComponentByClass<UPlayerStateRecorder>();
	StartTimer(this, GetWorld(), "ReattachMuzzleLocation", 0.7f, false);
}

void ABaseWeapon::SetupWeaponModi() {
	SoftAssertTrue(WeaponModi.Num(), GetWorld(), __FILE__, __LINE__, "Weapon has no weapon modules");
	for(int i = 0; i < WeaponModi.Num(); i++) {
		FBaseWeaponModus& modus = WeaponModi[i]; // use & to get a reference!
		// Apparently you need to set NewObject directly into a UPROPERTY()
		// setting a pointer returned from a method dose not work ^^
		modus.PrimaryModule = NewObject<UBaseWeaponModule>(this, modus.PrimaryModuleClass);
		modus.SecondaryModule = NewObject<UBaseWeaponModule>(this, modus.SecondaryModuleClass);
		AssertNotNull(modus.PrimaryModule, GetWorld(), __FILE__, __LINE__);
		if(modus.PrimaryModule) {
			modus.PrimaryModule->Weapon = this;
			modus.PrimaryModule->OwningCharacter = OwningCharacter;
			modus.PrimaryModule->Initialize();
		}
		if(modus.SecondaryModule) {
			modus.SecondaryModule->Weapon = this;
			modus.SecondaryModule->OwningCharacter = OwningCharacter;
			modus.SecondaryModule->Initialize();
		}
	}
}

bool ABaseWeapon::IsReadyForInitialization() {
	return OwningCharacter != nullptr;
}

void ABaseWeapon::ReattachMuzzleLocation() {
	// Reattach MuzzleLocation from weapon to camera
	AttachToOwner();
	FP_MuzzleLocation->AttachToComponent(OwningCharacter->FirstPersonCameraComponent, FAttachmentTransformRules(EAttachmentRule::KeepWorld, true));
	FP_Aim_MuzzleLocation->AttachToComponent(OwningCharacter->FirstPersonCameraComponent, FAttachmentTransformRules(EAttachmentRule::KeepWorld, true));
	if(!equipped) {
		OnUnequip();
	}
}

void ABaseWeapon::DetachFromOwner() {
	Mesh1P->DetachFromComponent(FDetachmentTransformRules::KeepRelativeTransform);
	Mesh3P->DetachFromComponent(FDetachmentTransformRules::KeepRelativeTransform);
	StopFire(EFireMode::Primary);
	StopFire(EFireMode::Secondary);
}

void ABaseWeapon::AttachToOwner() {
	DetachFromOwner();
	if(!OwningCharacter) {
		return;
	}
	Mesh1P->AttachToComponent(OwningCharacter->Mesh1P, FAttachmentTransformRules(EAttachmentRule::SnapToTarget, true), TEXT("GripPoint"));
	Mesh3P->AttachToComponent(OwningCharacter->Mesh3P, FAttachmentTransformRules(EAttachmentRule::SnapToTarget, true), TEXT("GripPoint"));
}

void ABaseWeapon::Tick(float DeltaTime) {
	Super::Tick(DeltaTime);
	if(HasAuthority()) {
		if(CurrentAmmoInClip == 0 && CurrentAmmo != 0 && CurrentState != EWeaponState::RELOADING
			&& CurrentState != EWeaponState::EQUIPPING /*&& timeSinceLastShot >= GetCurrentWeaponModus().ReloadDelay*/) {
			StartTimer(this, GetWorld(), "Reload", 0.1f, false); // use timer to avoid reload animation loops
		}
		if(GetCurrentWeaponModus().PrimaryModule)
			GetCurrentWeaponModus().PrimaryModule->AuthorityTick(DeltaTime);
		if(GetCurrentWeaponModus().SecondaryModule)
			GetCurrentWeaponModus().SecondaryModule->AuthorityTick(DeltaTime);
	}
	if(GetCurrentWeaponModus().PrimaryModule)
		GetCurrentWeaponModus().PrimaryModule->Tick(DeltaTime);
	if(GetCurrentWeaponModus().SecondaryModule)
		GetCurrentWeaponModus().SecondaryModule->Tick(DeltaTime);
}

void ABaseWeapon::StartFire(EFireMode mode) {
	if(!CanFire(mode))
		return;
	if(!GetCurrentWeaponModus().IsAsync && (GetCurrentWeaponModule(EFireMode::Primary)->IsFireing() || GetCurrentWeaponModule(EFireMode::Secondary)->IsFireing()))
		return;
	GetCurrentWeaponModule(mode)->StartFire();
	if(CurrentState == EWeaponState::IDLE && (GetCurrentWeaponModule(EFireMode::Primary)->IsFireing() || GetCurrentWeaponModule(EFireMode::Secondary)->IsFireing())) {
		ChangeWeaponState(EWeaponState::FIRING);
	}
}

void ABaseWeapon::StopFire(EFireMode mode) {
	GetCurrentWeaponModule(mode)->StopFire();
	if(CurrentState == EWeaponState::FIRING
		&& GetCurrentWeaponModule(EFireMode::Primary)->CurrentState == EWeaponModuleState::IDLE
		&& GetCurrentWeaponModule(EFireMode::Secondary)->CurrentState == EWeaponModuleState::IDLE) {
		ChangeWeaponState(EWeaponState::IDLE);
	}
}

void ABaseWeapon::FireOnce(EFireMode mode) {
	if(CanFire(mode)) // propably no check needed here ...
		GetCurrentWeaponModule(mode)->FireOnce();
}

bool ABaseWeapon::CanFire(EFireMode mode) {
	return true;
}

void ABaseWeapon::OnEquip() {
	ChangeWeaponState(EWeaponState::EQUIPPING);

	UpdateEquippedState(true);

	StartTimer(this, GetWorld(), "ChangeWeaponState", EquipDelay + 0.001f, false, EWeaponState::IDLE);
	GetCurrentWeaponModule(EFireMode::Primary)->OnActivate();
	GetCurrentWeaponModule(EFireMode::Secondary)->OnActivate();
}

void ABaseWeapon::OnUnequip() {
	GetCurrentWeaponModule(EFireMode::Primary)->StopFire();
	GetCurrentWeaponModule(EFireMode::Secondary)->StopFire();
	GetCurrentWeaponModule(EFireMode::Primary)->OnDeactivate();
	GetCurrentWeaponModule(EFireMode::Secondary)->OnDeactivate();
	UpdateEquippedState(false);
}

void ABaseWeapon::UpdateEquippedState(bool newEquipped) {
	this->equipped = newEquipped;
	SetActorTickEnabled(newEquipped);
	Mesh1P->SetHiddenInGame(!newEquipped, true);
	Mesh3P->SetHiddenInGame(!newEquipped, true);
	Mesh3P->bCastHiddenShadow = newEquipped;
	newEquipped ? AttachToOwner() : DetachFromOwner();
}

void ABaseWeapon::MulticastFireExecuted_Implementation() {
	//if(OwningCharacter && Cast<APlayerController>(OwningCharacter->GetController()))
	//	Cast<APlayerController>(OwningCharacter->GetController())->PlayerCameraManager->PlayCameraShake(CurrentWeaponModule->FireCameraShake, 1.0f);
	//SpawnMuzzleFX();
	//SpawnWeaponSound();
	//PlayFireAnimation();
	//OnFireExecuted.Broadcast();
}

void ABaseWeapon::SpawnWeaponSound(USoundBase* sound) {
	UGameplayStatics::SpawnSoundAttached(sound, Mesh1P);
}

void ABaseWeapon::MulticastSpawnNoAmmoSound_Implementation() {
	//UGameplayStatics::SpawnSoundAttached(NoAmmoSound, Mesh1P);
}

void ABaseWeapon::TogglePersistentSoundFX(UAudioComponent*& component, class USoundBase* soundClass, bool shouldPlay, float fadeOut) {
	if(shouldPlay) {
		if(!component) {
			component = UGameplayStatics::SpawnSoundAttached(soundClass, Mesh1P, NAME_None, FVector(ForceInit), FRotator::ZeroRotator, EAttachLocation::KeepRelativeOffset, false, 1.f, 1.f, 0.f, nullptr, nullptr, false); // bAutoDestroy=false
		} else if(!component->IsPlaying()) {
			component->Play(0);
		}
	} else if(component && component->IsPlaying()) {
		component->FadeOut(fadeOut, 0);
	}
}

void ABaseWeapon::PlayFireAnimation() {
	UAnimInstance* AnimInstance = OwningCharacter->Mesh1P->GetAnimInstance();
	if(AnimInstance != NULL) {
		AnimInstance->Montage_Play(FireAnimation, 1.f);
	}
	Mesh1P->PlayAnimation(FireAnimationWeapon1P, false);
}

void ABaseWeapon::Reload() {
	if(CurrentState != EWeaponState::RELOADING && CurrentState != EWeaponState::EQUIPPING && CurrentAmmoInClip != ClipSize && CurrentAmmoInClip != CurrentAmmo) {
		OwningCharacter->Mesh1P->GetAnimInstance()->OnPlayMontageNotifyBegin.AddUnique(AnimationNotifyDelegate);

		ChangeWeaponState(EWeaponState::RELOADING);

		GetCurrentWeaponModule(EFireMode::Primary)->OnDeactivate();
		GetCurrentWeaponModule(EFireMode::Secondary)->OnDeactivate();

		MulticastPlayReloadAnimation();
		OwningCharacter->Mesh1P->GetAnimInstance()->Montage_Play(ReloadAnimation1P);
		OwningCharacter->Mesh3P->GetAnimInstance()->Montage_Play(ReloadAnimation3P);
		Mesh1P->PlayAnimation(ReloadAnimationWeapon1P, false);
	}
}

void ABaseWeapon::MulticastPlayReloadAnimation_Implementation() {
	if(!HasAuthority()) {
		OwningCharacter->Mesh1P->GetAnimInstance()->Montage_Play(ReloadAnimation1P);
		OwningCharacter->Mesh3P->GetAnimInstance()->Montage_Play(ReloadAnimation3P);
		Mesh1P->PlayAnimation(ReloadAnimationWeapon1P, false);
	}
}

void ABaseWeapon::ReloadAnimationNotifyCallback(FName NotifyName, const FBranchingPointNotifyPayload& BranchingPointPayload) {
	if(NotifyName.ToString() == "RefillAmmo") {
		CurrentAmmoInClip = CurrentAmmo < 0 ? ClipSize : FMath::Min(CurrentAmmo, ClipSize);
		ForceNetUpdate();
	} else if(NotifyName.ToString() == "EnableFiring") {
		ChangeWeaponState(EWeaponState::IDLE);
		GetCurrentWeaponModule(EFireMode::Primary)->OnActivate();
		GetCurrentWeaponModule(EFireMode::Secondary)->OnActivate();
		if(GetCurrentWeaponModule(EFireMode::Primary)->IsFireing() || GetCurrentWeaponModule(EFireMode::Secondary)->IsFireing()) {
			ChangeWeaponState(EWeaponState::FIRING);
		}
	}
}

void ABaseWeapon::SpawnMuzzleFX(UParticleSystem* muzzleFx, float duration, FVector scale) {
	UParticleSystemComponent* mesh1pFX = UGameplayStatics::SpawnEmitterAttached(muzzleFx, Mesh1P, NAME_None);
	if(mesh1pFX) {
		mesh1pFX->SetRelativeScale3D(scale);
		mesh1pFX->SetWorldLocation(GetFPMuzzleLocation());
		mesh1pFX->SetWorldRotation(GetAimDirection());
		mesh1pFX->bOwnerNoSee = false;
		mesh1pFX->bOnlyOwnerSee = true;
		if(duration > 0)
			StartTimer(this, GetWorld(), "DecativateParticleSystem", duration, false, mesh1pFX);
	}
	UParticleSystemComponent* mesh3pFX = UGameplayStatics::SpawnEmitterAttached(muzzleFx, Mesh3P, NAME_None);
	if(mesh3pFX) {
		mesh3pFX->SetRelativeScale3D(scale);
		mesh3pFX->SetWorldLocation(GetTPMuzzleLocation());
		mesh3pFX->SetWorldRotation(GetAimDirection());
		mesh3pFX->bOwnerNoSee = true;
		mesh3pFX->bOnlyOwnerSee = false;
		if(duration > 0)
			StartTimer(this, GetWorld(), "DecativateParticleSystem", duration, false, mesh3pFX);
	}
}

void ABaseWeapon::DecativateParticleSystem(UParticleSystemComponent* effect) {
	if(effect)
		effect->DeactivateSystem();
}

FRotator ABaseWeapon::GetAimDirection() {
	if(OwningCharacter)
		return OwningCharacter->FirstPersonCameraComponent->GetComponentRotation();
	return FRotator();
}

FVector ABaseWeapon::GetFPMuzzleLocation() {
	if(OwningCharacter && OwningCharacter->IsAiming) {
		return FP_Aim_MuzzleLocation->GetComponentLocation();
	}
	return FP_MuzzleLocation->GetComponentLocation();
}

FVector ABaseWeapon::GetTPMuzzleLocation() {
	return TP_MuzzleLocation->GetComponentLocation();
}

void ABaseWeapon::StartAiming() {
	if(HideWeaponWhenAiming) {
		Mesh1P->SetVisibility(false, true);
		if(OwningCharacter)
			OwningCharacter->Mesh1P->SetVisibility(false, false);
	}
}

void ABaseWeapon::StopAiming() {
	Mesh1P->SetVisibility(true, true);
	if(OwningCharacter)
		OwningCharacter->Mesh1P->SetVisibility(true, false);
}

void ABaseWeapon::ChangeWeaponState(EWeaponState newState) {
	CurrentState = newState;
	MulticastStateChanged(newState);
}

void ABaseWeapon::MulticastStateChanged_Implementation(EWeaponState newState) {
	OnStateChanged(newState);
}

EWeaponState ABaseWeapon::GetCurrentWeaponState() {
	return CurrentState;
}

float ABaseWeapon::GetAIWeaponSuitability(ABaseCharacter* shooter, AActor* victim) {
	if(CurrentAmmo == 0)
		return 0;
	float distance = 1000;
	if(shooter && victim)
		distance = (shooter->GetActorLocation() - victim->GetActorLocation()).Size();
	return AISuitabilityWeaponRangeCurve.GetRichCurveConst()->Eval(distance, 0);
	//return 1.0;
}

bool ABaseWeapon::ServerIncreaseCurrentAmmo_Validate(int amount) {
	return true;
}

void ABaseWeapon::ServerIncreaseCurrentAmmo_Implementation(int amount) {
	if(MaxAmmo >= 0 && CurrentAmmo + amount > MaxAmmo) {
		CurrentAmmo = MaxAmmo;
		return;
	}
	if(CurrentAmmo <= -1)
		return;
	CurrentAmmo += amount;
}

FBaseWeaponModus& ABaseWeapon::GetCurrentWeaponModus() {
	int index = FMath::Clamp(CurrentWeaponModusIndex, 0, WeaponModi.Num() - 1);
	return WeaponModi[index];
}


UBaseWeaponModule* ABaseWeapon::GetCurrentWeaponModule(EFireMode mode) {
	auto module = mode == EFireMode::Primary ?
		GetCurrentWeaponModus().PrimaryModule :
		GetCurrentWeaponModus().SecondaryModule;
	AssertNotNull(module, GetWorld(), __FILE__, __LINE__);
	return module;
}
