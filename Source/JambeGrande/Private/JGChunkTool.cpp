// Fill out your copyright notice in the Description page of Project Settings.


#include "JGChunkTool.h"

#include "EditorUtilityLibrary.h"
#include "FileHelpers.h"
#include "JGChunk.h"
#include "Engine/Blueprint.h"

UJGChunkTool::UJGChunkTool()
{
	SupportedClasses.Add(UBlueprint::StaticClass());
}

void UJGChunkTool::SetupChunk()
{
	SetupWallBoxCollisionOnSelected();
	SetupTriggerBoxOnSelected();
	SetupFloorOnSelected();
}

void UJGChunkTool::ProcessSelectedChunkBlueprints(TFunction<void(AJGChunk*)> setupFunction, const FString& operationName)
{
	TArray<UObject*> selectedAssets = UEditorUtilityLibrary::GetSelectedAssets();
	
	for (UObject* selectedAsset : selectedAssets)
	{
		if (IsValid(selectedAsset))
		{
			bool hasChanged = false;
			
			// Try to get the Blueprint's generated class
			UBlueprint* blueprint = Cast<UBlueprint>(selectedAsset);
			if (IsValid(blueprint) && IsValid(blueprint->GeneratedClass))
			{
				// Check if the Blueprint's generated class is based on AJGChunk
				if (blueprint->GeneratedClass->IsChildOf<AJGChunk>())
				{
					// Get the default object (CDO) of the chunk class
					AJGChunk* chunkCDO = Cast<AJGChunk>(blueprint->GeneratedClass->GetDefaultObject());
					if (IsValid(chunkCDO))
					{
						setupFunction(chunkCDO);
						hasChanged = true;
						UE_LOG(LogTemp, Log, TEXT("Called %s on chunk blueprint: %s"), *operationName, *selectedAsset->GetName());
					}
				}
				else
				{
					UE_LOG(LogTemp, Warning, TEXT("Selected blueprint is not based on AJGChunk: %s"), *selectedAsset->GetName());
				}
			}
			else
			{
				UE_LOG(LogTemp, Warning, TEXT("Selected asset is not a Blueprint: %s"), *selectedAsset->GetName());
			}

			if (hasChanged)
			{
				selectedAsset->MarkPackageDirty();
				UE_LOG(LogTemp, Log, TEXT("Marked package as dirty for %s: %s"), *operationName, *selectedAsset->GetName());
			
				// Save the modified asset
				TArray<UPackage*> packagesToSave;
				packagesToSave.Add(selectedAsset->GetPackage());
				FEditorFileUtils::PromptForCheckoutAndSave(packagesToSave, false, false);
			
				UE_LOG(LogTemp, Log, TEXT("%s completed and saved for chunk"), *operationName);
			}
		}
	}
}

void UJGChunkTool::SetupFloorOnSelected()
{
	ProcessSelectedChunkBlueprints([](AJGChunk* chunk)
	{
		chunk->SetupFloor();
	}, TEXT("SetupFloor"));
}

void UJGChunkTool::SetupWallBoxCollisionOnSelected()
{
	ProcessSelectedChunkBlueprints([](AJGChunk* chunk)
	{
		chunk->SetupWallBoxCollision();
	}, TEXT("SetupWallBoxCollision"));
}

void UJGChunkTool::SetupTriggerBoxOnSelected()
{
	ProcessSelectedChunkBlueprints([](AJGChunk* chunk)
	{
		chunk->SetupTriggerBox();
	}, TEXT("SetupTriggerBox"));
}