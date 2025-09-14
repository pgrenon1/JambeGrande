// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "Public/JGNavMeshManager.h"
#include "EnferGameMode.generated.h"

class UUserWidget;

/**
 *  Simple GameMode for a first person game
 */
UCLASS(abstract)
class AEnferGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	AEnferGameMode();

	virtual bool SetPause(APlayerController* playerController, FCanUnpause canUnpauseDelegate = FCanUnpause()) override;
	virtual bool ClearPause() override;

	UFUNCTION(BlueprintCallable, Category = "GameMode")
	void BP_ClearPause();
	
protected:
	// Nav mesh manager component
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Navigation")
	UJGNavMeshManager* NavMeshManager;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Pause")
	UUserWidget* PauseMenuInstance;
};



