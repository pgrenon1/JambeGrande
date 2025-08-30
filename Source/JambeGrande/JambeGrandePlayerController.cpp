// Copyright Epic Games, Inc. All Rights Reserved.


#include "JambeGrandePlayerController.h"
#include "EnhancedInputSubsystems.h"
#include "Engine/LocalPlayer.h"
#include "InputMappingContext.h"
#include "JambeGrandeCameraManager.h"

AJambeGrandePlayerController::AJambeGrandePlayerController()
{
	// set the player camera manager class
	PlayerCameraManagerClass = AJambeGrandeCameraManager::StaticClass();
}

void AJambeGrandePlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	// Add Input Mapping Context
	if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
	{
		for (UInputMappingContext* CurrentContext : DefaultMappingContexts)
		{
			Subsystem->AddMappingContext(CurrentContext, 0);
		}
	}
}
