// Copyright Epic Games, Inc. All Rights Reserved.

#include "EnferGameMode.h"

#include "Blueprint/UserWidget.h"
#include "Components/ActorComponent.h"

AEnferGameMode::AEnferGameMode()
{
	// Create nav mesh manager component
	NavMeshManager = CreateDefaultSubobject<UJGNavMeshManager>(TEXT("NavMeshManager"));
}

bool AEnferGameMode::SetPause(APlayerController* playerController, FCanUnpause canUnpauseDelegate)
{
	bool isPauseSet = Super::SetPause(playerController, canUnpauseDelegate);
	
	if (IsValid(PauseMenuInstance))
		PauseMenuInstance->SetVisibility(ESlateVisibility::Visible);

	if (isPauseSet)
		GetWorldSettings()->SetTimeDilation(0.0f);

	GetWorld()->GetFirstPlayerController()->SetShowMouseCursor(true);
	GetWorld()->GetFirstPlayerController()->SetInputMode(FInputModeUIOnly());
	
	return isPauseSet;
}

bool AEnferGameMode::ClearPause()
{
	bool isPauseCleared = Super::ClearPause();

	if (isPauseCleared)
		GetWorldSettings()->SetTimeDilation(1.0f);

	if (IsValid(PauseMenuInstance))
		PauseMenuInstance->SetVisibility(ESlateVisibility::Collapsed);

	GetWorld()->GetFirstPlayerController()->SetShowMouseCursor(false);
	GetWorld()->GetFirstPlayerController()->SetInputMode(FInputModeGameOnly());

	return isPauseCleared;
}

void AEnferGameMode::BP_ClearPause()
{
	ClearPause();
}
