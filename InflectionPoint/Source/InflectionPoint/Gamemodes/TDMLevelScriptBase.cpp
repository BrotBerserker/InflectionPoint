// Fill out your copyright notice in the Description page of Project Settings.

#include "InflectionPoint.h"
#include "TDMLevelScriptBase.h"
#include "Gameplay/Characters/BaseCharacter.h"
#include "Runtime/LevelSequence/Public/LevelSequenceActor.h"
#include "Gamemodes/TDMPlayerStateBase.h"


void ATDMLevelScriptBase::MulticastStartSpawnCinematic_Implementation() {DebugPrint(__FILE__, __LINE__);
	if(!SoftAssertTrue(SpawnCinematicLevelSequences.Num() > GetTeam() && GetTeam() >= 0, GetWorld(), __FILE__, __LINE__, "SpawnSequence could not be played!")) {DebugPrint(__FILE__, __LINE__);
		return;
	}
	OnPrePlaySpawnCinematic();
	PrepareAndStartSequence(SpawnCinematicCamera, SpawnCinematicLevelSequences[GetTeam()], 0.3f);
}

void ATDMLevelScriptBase::StartEndMatchSequence(TSubclassOf<AActor> WinningActor, TSubclassOf<AActor> LosingActor, FString WinnerName, FString LoserName) {DebugPrint(__FILE__, __LINE__);
	if(SoftAssertTrue(WinningPlayerLocation != nullptr && LosingPlayerLocation != nullptr, GetWorld(), __FILE__, __LINE__, "No locations for winning/losing player provided!")) {DebugPrint(__FILE__, __LINE__);
		AActor* winner = SpawnActorForEndMatchSequence(WinningActor, WinningPlayerLocation);
		AActor* loser = SpawnActorForEndMatchSequence(LosingActor, LosingPlayerLocation);
		PrepareActorForEndMatchSequence(winner, WinningPlayerAnimation);
		PrepareActorForEndMatchSequence(loser, LosingPlayerAnimation);
	}
	MulticastStartEndMatchSequence(WinnerName, LoserName);
}

void ATDMLevelScriptBase::MulticastStartEndMatchSequence_Implementation(const FString& winnerName, const FString& loserName) {DebugPrint(__FILE__, __LINE__);
	// TODO disable HUD
	UpdateNameTag(WinningPlayerLocation, winnerName);
	UpdateNameTag(LosingPlayerLocation, loserName);
	PrepareAndStartSequence(MatchEndCamera, MatchEndLevelSequence, 1.f);
}

void ATDMLevelScriptBase::PrepareAndStartSequence(ACameraActor* camera, ALevelSequenceActor* sequenceActor, float fadeTime) {DebugPrint(__FILE__, __LINE__);
	if(!SoftAssertTrue(sequenceActor != nullptr, GetWorld(), __FILE__, __LINE__, "No level sequence provided!"))
		return;
	APlayerController* controller = UGameplayStatics::GetPlayerController(GetWorld(), 0);
	controller->ClientSetCameraFade(true, FColor::Black, FVector2D(0, 1), fadeTime + 0.1f);
	StartTimer(this, GetWorld(), "StartSequence", fadeTime, false, controller, camera, sequenceActor, fadeTime);
}

void ATDMLevelScriptBase::StartSequence(APlayerController* controller, ACameraActor* camera, ALevelSequenceActor* sequenceActor, float fadeTime) {DebugPrint(__FILE__, __LINE__);
	controller->ClientSetCameraFade(true, FColor::Black, FVector2D(1, 0), fadeTime);
	controller->SetViewTargetWithBlend(camera);
	sequenceActor->SequencePlayer->SetPlaybackPosition(0);
	sequenceActor->SequencePlayer->Play();
	sequenceActor->SequencePlayer->OnStop.AddDynamic(this, &ATDMLevelScriptBase::CleanUpAfterSequence);
}

AActor* ATDMLevelScriptBase::SpawnActorForEndMatchSequence(TSubclassOf<AActor> actorToSpawn, AActor* location) {DebugPrint(__FILE__, __LINE__);
	FVector loc = location->GetTransform().GetLocation();
	FRotator rot = FRotator(location->GetTransform().GetRotation());

	FActorSpawnParameters ActorSpawnParams;
	ActorSpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	return GetWorld()->SpawnActor<AActor>(actorToSpawn, loc, rot, ActorSpawnParams);
}

void ATDMLevelScriptBase::PrepareActorForEndMatchSequence(AActor * spawnedActor, UAnimationAsset* animation) {DebugPrint(__FILE__, __LINE__);
	ABaseCharacter* baseCharacter = Cast<ABaseCharacter>(spawnedActor);
	if(baseCharacter) {DebugPrint(__FILE__, __LINE__);
		baseCharacter->ServerEquipRandomWeapon();
		baseCharacter->MulticastPlay3PAnimation(animation);
	}
}

void ATDMLevelScriptBase::UpdateNameTag(AActor* location, FString name) {DebugPrint(__FILE__, __LINE__);
	if(!location) {DebugPrint(__FILE__, __LINE__);
		return;
	}
	UTextRenderComponent* textRender = location->FindComponentByClass<UTextRenderComponent>();
	if(textRender) {DebugPrint(__FILE__, __LINE__);
		textRender->SetText(FText::FromString(name));
	}
}

int ATDMLevelScriptBase::GetTeam() {DebugPrint(__FILE__, __LINE__);
	auto controller = UGameplayStatics::GetPlayerController(GetWorld(), 0);
	if(!controller) {DebugPrint(__FILE__, __LINE__);
		return -1;
	}
	auto state = Cast<ATDMPlayerStateBase>(controller->PlayerState);
	if(!state) {DebugPrint(__FILE__, __LINE__);
		return -1;
	}
	return state->Team;
}

void ATDMLevelScriptBase::CleanUpAfterSequence() {DebugPrint(__FILE__, __LINE__);
	APlayerController* controller = UGameplayStatics::GetPlayerController(GetWorld(), 0);
	controller->SetViewTargetWithBlend(controller->GetCharacter());
	UpdateNameTag(WinningPlayerLocation, "");
	UpdateNameTag(LosingPlayerLocation, "");
}
