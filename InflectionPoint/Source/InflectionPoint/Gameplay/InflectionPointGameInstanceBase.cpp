// Fill out your copyright notice in the Description page of Project Settings.

#include "InflectionPoint.h"
#include "InflectionPointGameInstanceBase.h"


UInflectionPointGameInstanceBase::UInflectionPointGameInstanceBase() {
	IPSettings = CreateDefaultSubobject<UInflectionPointSettings>(TEXT("IPSettings"));
}

