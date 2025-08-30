// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AssetActionUtility.h"
#include "JGChunkTool.generated.h"

class AJGChunk;
/**
 * 
 */
UCLASS()
class JAMBEGRANDE_API UJGChunkTool : public UAssetActionUtility
{
	GENERATED_BODY()

public:
	UJGChunkTool();

	UFUNCTION(CallInEditor, Category = "Chunk Tool")
	void SetupChunk();

	UFUNCTION(CallInEditor, Category = "Chunk Tool")
	void SetupFloorOnSelected();

	UFUNCTION(CallInEditor, Category = "Chunk Tool")
	void SetupWallBoxCollisionOnSelected();

	UFUNCTION(CallInEditor, Category = "Chunk Tool")
	void SetupTriggerBoxOnSelected();

private:
	// Helper method to process selected chunk blueprints
	static void ProcessSelectedChunkBlueprints(TFunction<void(AJGChunk*)> setupFunction, const FString& operationName);
};
