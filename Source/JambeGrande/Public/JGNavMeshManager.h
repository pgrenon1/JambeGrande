// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "NavigationSystem.h"
#include "NavMesh/NavMeshBoundsVolume.h"
#include "JGLevelGenerator.h"
#include "JGNavMeshManager.generated.h"

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class JAMBEGRANDE_API UJGNavMeshManager : public UActorComponent
{
	GENERATED_BODY()

public:
	UJGNavMeshManager();

	// Number of chunks before and after the player's current chunk that should have nav mesh coverage
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Navigation", meta = (ClampMin = "1"))
	int32 ChunkBufferSize = 2;

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	// Current center chunk index for nav mesh generation
	UPROPERTY(Transient)
	int32 CurrentCenterChunkIndex;

	// Reference to the level generator (automatically found and bound)
	UPROPERTY(Transient)
	UJGLevelGenerator* LevelGenerator;

	// Reference to the navigation system
	UPROPERTY(Transient)
	UNavigationSystemV1* NavigationSystem;

	// Reference to the nav mesh bounds volume (automatically found)
	UPROPERTY(Transient)
	ANavMeshBoundsVolume* NavMeshBoundsVolume;

	// Box extent for nav mesh rebuilding
	FBox CurrentNavMeshBounds;

public:
	// Called when player enters a new chunk
	UFUNCTION()
	void OnPlayerEnteredChunk(int32 newChunkIndex, int32 previousChunkIndex);

	// Manually update nav mesh for specific chunk range
	UFUNCTION(BlueprintCallable, Category = "Navigation")
	void UpdateNavMeshForChunkRange(int32 centerChunkIndex, int32 bufferSize);

	// Get the current nav mesh bounds
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Navigation")
	FBox GetCurrentNavMeshBounds() const { return CurrentNavMeshBounds; }

private:
	// Find and bind to the level generator component
	void FindAndBindToLevelGenerator();

	// Find the nav mesh bounds volume in the world
	void FindNavMeshBoundsVolume();

	// Calculate nav mesh bounds for given chunk range
	FBox CalculateNavMeshBounds(int32 centerChunkIndex, int32 bufferSize);

	// Update the nav mesh bounds volume and trigger rebuild
	void UpdateNavMeshBounds(const FBox& newBounds);
};