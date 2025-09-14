// Fill out your copyright notice in the Description page of Project Settings.

#include "Public/JGNavMeshManager.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "NavigationSystem.h"
#include "NavMesh/RecastNavMesh.h"
#include "NavMesh/NavMeshBoundsVolume.h"
#include "Components/ActorComponent.h"
#include "Components/BrushComponent.h"
#include "Kismet/GameplayStatics.h"

UJGNavMeshManager::UJGNavMeshManager()
{
	PrimaryComponentTick.bCanEverTick = false;
	CurrentCenterChunkIndex = 0;
	NavigationSystem = nullptr;
	NavMeshBoundsVolume = nullptr;
	CurrentNavMeshBounds = FBox(ForceInit);
}

void UJGNavMeshManager::BeginPlay()
{
	Super::BeginPlay();

	// Get navigation system reference
	NavigationSystem = FNavigationSystem::GetCurrent<UNavigationSystemV1>(GetWorld());
	if (!IsValid(NavigationSystem))
	{
		UE_LOG(LogTemp, Warning, TEXT("JGNavMeshManager: Navigation system not found!"));
		return;
	}

	// Find and bind to level generator
	FindAndBindToLevelGenerator();

	// Find the nav mesh bounds volume
	FindNavMeshBoundsVolume();

	// Initialize nav mesh for starting chunks
	UpdateNavMeshForChunkRange(CurrentCenterChunkIndex, ChunkBufferSize);
}

void UJGNavMeshManager::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	// Unbind from level generator if bound
	if (IsValid(LevelGenerator))
	{
		LevelGenerator->OnPlayerEnteredChunkDelegate.RemoveDynamic(this, &UJGNavMeshManager::OnPlayerEnteredChunk);
	}

	Super::EndPlay(EndPlayReason);
}

void UJGNavMeshManager::FindAndBindToLevelGenerator()
{
	// Look for level generator in the same actor first
	UJGLevelGenerator* levelGenerator = GetOwner()->FindComponentByClass<UJGLevelGenerator>();

	if (IsValid(levelGenerator))
	{
		LevelGenerator = levelGenerator;
		LevelGenerator->OnPlayerEnteredChunkDelegate.AddDynamic(this, &UJGNavMeshManager::OnPlayerEnteredChunk);
		UE_LOG(LogTemp, Log, TEXT("JGNavMeshManager: Successfully found and bound to level generator"));
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("JGNavMeshManager: Could not find level generator component!"));
	}
}

void UJGNavMeshManager::FindNavMeshBoundsVolume()
{
	UWorld* world = GetWorld();
	if (!IsValid(world))
	{
		UE_LOG(LogTemp, Warning, TEXT("JGNavMeshManager: World not found for NavMeshBoundsVolume search"));
		return;
	}

	// Find NavMeshBoundsVolume using GameplayStatics
	NavMeshBoundsVolume = Cast<ANavMeshBoundsVolume>(UGameplayStatics::GetActorOfClass(world, ANavMeshBoundsVolume::StaticClass()));
	
	if (IsValid(NavMeshBoundsVolume))
	{
		UE_LOG(LogTemp, Log, TEXT("JGNavMeshManager: Found NavMeshBoundsVolume"));
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("JGNavMeshManager: No NavMeshBoundsVolume found in the world! Nav mesh will not be properly constrained."));
	}
}

void UJGNavMeshManager::OnPlayerEnteredChunk(int32 newChunkIndex, int32 previousChunkIndex)
{
	if (!IsValid(NavigationSystem) || !IsValid(LevelGenerator))
	{
		return;
	}

	UE_LOG(LogTemp, Log, TEXT("JGNavMeshManager: Player entered chunk %d (from %d)"), newChunkIndex, previousChunkIndex);

	// Update center chunk index to the new player chunk
	CurrentCenterChunkIndex = newChunkIndex;

	// Update nav mesh for the new chunk range
	UpdateNavMeshForChunkRange(CurrentCenterChunkIndex, ChunkBufferSize);
}

void UJGNavMeshManager::UpdateNavMeshForChunkRange(int32 centerChunkIndex, int32 bufferSize)
{
	if (!IsValid(NavigationSystem) || !IsValid(LevelGenerator))
	{
		UE_LOG(LogTemp, Warning, TEXT("JGNavMeshManager: Cannot update nav mesh - missing navigation system or level generator"));
		return;
	}

	// Calculate new bounds for nav mesh
	FBox newBounds = CalculateNavMeshBounds(centerChunkIndex, bufferSize);
	
	if (newBounds.IsValid)
	{
		UpdateNavMeshBounds(newBounds);
		UE_LOG(LogTemp, Log, TEXT("JGNavMeshManager: Updated nav mesh for chunk range [%d] with buffer %d"), 
			centerChunkIndex, bufferSize);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("JGNavMeshManager: Invalid bounds calculated for nav mesh update"));
	}
}

FBox UJGNavMeshManager::CalculateNavMeshBounds(int32 centerChunkIndex, int32 bufferSize)
{
	if (!IsValid(LevelGenerator))
	{
		return FBox(ForceInit);
	}

	// Use the level generator's improved bounds calculation
	return LevelGenerator->GetChunkRangeBounds(centerChunkIndex, bufferSize);
}

void UJGNavMeshManager::UpdateNavMeshBounds(const FBox& newBounds)
{
	if (!IsValid(NavigationSystem))
	{
		return;
	}

	CurrentNavMeshBounds = newBounds;

	// Update the NavMeshBoundsVolume if we found one
	if (IsValid(NavMeshBoundsVolume))
	{
		// Calculate the center and extent of the new bounds
		FVector center = newBounds.GetCenter();
		FVector extent = newBounds.GetExtent();

		// Set the actor location to the center of the bounds
		NavMeshBoundsVolume->SetActorLocation(center);

		// Get the brush component and update its scale
		UBrushComponent* brushComponent = NavMeshBoundsVolume->GetBrushComponent();
		if (IsValid(brushComponent))
		{
			// The default brush is a 200x200x200 unit cube, so we need to scale to our desired extent
			FVector defaultSize = FVector(200.0f, 200.0f, 200.0f);
			FVector newScale = (extent * 2.0f) / defaultSize; // extent * 2 because extent is half-size
			
			NavMeshBoundsVolume->SetActorScale3D(newScale);
			NavigationSystem->OnNavigationBoundsUpdated(NavMeshBoundsVolume);
			
			UE_LOG(LogTemp, Log, TEXT("JGNavMeshManager: Updated NavMeshBoundsVolume - Center: %s, Scale: %s"), 
				*center.ToString(), *newScale.ToString());
		}

		// Request nav mesh rebuild for the specified bounds
		NavigationSystem->AddDirtyArea(newBounds, ENavigationDirtyFlag::All);
		NavigationSystem->OnNavigationBoundsUpdated(NavMeshBoundsVolume);
		
		UE_LOG(LogTemp, Log, TEXT("JGNavMeshManager: Requested nav mesh rebuild for bounds: %s"), 
			*newBounds.ToString());
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("JGNavMeshManager: No NavMeshBoundsVolume available for resizing"));
		
		// Fallback: just request dirty area rebuild
		NavigationSystem->AddDirtyArea(newBounds, ENavigationDirtyFlag::All);
		NavigationSystem->OnNavigationBoundsUpdated(NavMeshBoundsVolume);
	}
}