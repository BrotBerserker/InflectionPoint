// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Countdown.generated.h"


//DECLARE_DYNAMIC_DELEGATE_OneParam(FOnUpdateDelegate, int32, asd);
DECLARE_DELEGATE_OneParam(FOnUpdateDelegate, int32);
DECLARE_DELEGATE(FOnFinishDelegate);

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class INFLECTIONPOINT_API UCountdown : public UActorComponent {
	GENERATED_BODY()


public:
	FOnUpdateDelegate OnUpdate;
	FOnFinishDelegate OnFinish;


public:
	UCountdown();

	virtual void BeginPlay() override;

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	template <class clars>
	using FPointerInt = void(clars::*)(int);

	template <class clars>
	using FPointer = void(clars::*)();

	template <class MegaClass>
	void Setup(MegaClass* Obj, FPointerInt<MegaClass> UpdateFunction, FPointer<MegaClass> FinishFunction, int CountdownDuration, bool FirstUpdateInstant = false) {
		OnUpdate.BindUObject(Obj, UpdateFunction);
		OnFinish.BindUObject(Obj, FinishFunction);
		Duration = CountdownDuration;
		InstantUpdate = FirstUpdateInstant;
	}

	void Start();

	void Stop();

private:
	int Duration;
	int RemainingSeconds = -1;
	float TimeUntilNextUpdate = 1.f;
	bool InstantUpdate = false;

};
