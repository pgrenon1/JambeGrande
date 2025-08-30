// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "Public/JGNavMeshManager.h"
#include "JambeGrandeGameMode.generated.h"

/**
 *  Simple GameMode for a first person game
 */
UCLASS(abstract)
class AJambeGrandeGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	AJambeGrandeGameMode();

protected:
	// Nav mesh manager component
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Navigation")
	UJGNavMeshManager* NavMeshManager;
};



