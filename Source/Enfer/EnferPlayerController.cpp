// Copyright Epic Games, Inc. All Rights Reserved.


#include "EnferPlayerController.h"
#include "EnhancedInputSubsystems.h"
#include "Engine/LocalPlayer.h"
#include "InputMappingContext.h"
#include "EnferCameraManager.h"

AEnferPlayerController::AEnferPlayerController()
{
	// set the player camera manager class
	PlayerCameraManagerClass = AEnferCameraManager::StaticClass();
}

void AEnferPlayerController::SetupInputComponent()
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
