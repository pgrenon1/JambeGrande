// Copyright Epic Games, Inc. All Rights Reserved.

#include "JambeGrandeGameMode.h"
#include "Components/ActorComponent.h"

AJambeGrandeGameMode::AJambeGrandeGameMode()
{
	// Create nav mesh manager component
	NavMeshManager = CreateDefaultSubobject<UJGNavMeshManager>(TEXT("NavMeshManager"));
}
