// Fill out your copyright notice in the Description page of Project Settings.


#include "Public/JGLevelGenerator.h"

#include "JGNPC.h"
#include "Engine/Engine.h"
#include "GameFramework/PlayerStart.h"
#include "Kismet/GameplayStatics.h"

UJGLevelGenerator::UJGLevelGenerator()
{
	PrimaryComponentTick.bCanEverTick = false;
	FrontActor = nullptr;
	BackActor = nullptr;
	MirrorYOffset = 500.0f;
}

void UJGLevelGenerator::BeginPlay()
{
	Super::BeginPlay();

	SpawnInitialChunks();
}

void UJGLevelGenerator::OnPlayerEnteredChunk(int32 newChunkIndex, int32 previousChunkIndex)
{
	UE_LOG(LogTemp, Log, TEXT("Player entered chunk index: %d -> %d"), previousChunkIndex, newChunkIndex);

	// Update the current chunk index
	PlayerCurrentChunkIndex = newChunkIndex;

	// Determine if the player has moved forward or backward
	bool forward = newChunkIndex > previousChunkIndex;

	DespawnExtremityChunk(!forward);
	SpawnChunk(forward);

	// Broadcast the delegate for other systems (like nav mesh manager) to respond
	OnPlayerEnteredChunkDelegate.Broadcast(newChunkIndex, previousChunkIndex);
}

void UJGLevelGenerator::SpawnChunk(bool forward)
{
	if (ChunkClasses.Num() == 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("No chunk visuals classes defined! Cannot spawn new chunk."));
		return;		
	}

	TSubclassOf<AJGChunk> chunkClass = ChunkClasses[FMath::RandRange(0, ChunkClasses.Num() - 1)];
	
	FVector newLocation = FVector::ZeroVector;
	int32 logicalIndex = 0;
	
	AJGChunk* newChunk = GetWorld()->SpawnActorDeferred<AJGChunk>(chunkClass, FTransform::Identity);
	if (!newChunk)
		return;

	FVector extent = FVector::ZeroVector;
	FVector location = FVector::ZeroVector;
	newChunk->GetActorBounds(true, location, extent, true);

	FChunkData extremityChunkData = GetExtremityChunkData(forward);
	if (extremityChunkData.IsValid()) // if it's the first chunk, extremityChunkData will be invalid and this part is skipped
	{
		FVector extremityChunkLocation = extremityChunkData.ChunkActor->GetActorLocation();
		logicalIndex = forward ? extremityChunkData.ChunkActor->ChunkLogicalIndex + 1 : extremityChunkData.ChunkActor->ChunkLogicalIndex - 1;
		newLocation = extremityChunkLocation + FVector(forward ? extremityChunkData.ActorExtents.X * 2 : -extent.X * 2, 0, 0);
	}

	newChunk->SetIndex(logicalIndex);
	newChunk->FinishSpawning(FTransform(newLocation));

	// Recalculate bounds after spawning to ensure accurate extents
	newChunk->GetActorBounds(true, location, extent, true);

	// Spawn the mirror chunk: rotate 180 degrees about Z and offset on Y
	FRotator mirrorRotation(0.0f, 180.0f, 0.0f);
	FVector mirrorLocation = newLocation + FVector(extent.X * 2, MirrorYOffset, 0.0f);
	FTransform mirrorTransform(mirrorRotation, mirrorLocation);

	AJGChunk* mirrorChunk = GetWorld()->SpawnActorDeferred<AJGChunk>(chunkClass, mirrorTransform);
	if (IsValid(mirrorChunk))
	{
		mirrorChunk->SetIndex(logicalIndex);
		mirrorChunk->FinishSpawning(mirrorTransform);
		mirrorChunk->SetActorLabel(newChunk->GetActorLabel() + TEXT("_Mirror"));
	}

	if (forward)
		ActiveChunks.Add(FChunkData(newChunk, mirrorChunk, extent));
	else
		ActiveChunks.Insert(FChunkData(newChunk, mirrorChunk, extent), 0);
}

void UJGLevelGenerator::DespawnExtremityChunk(bool forward)
{
	FChunkData chunkData = GetExtremityChunkData(forward);
	AJGChunk* chunkToRemove = chunkData.ChunkActor;
	AJGChunk* mirrorChunkToRemove = chunkData.MirrorChunkActor;
	
	chunkToRemove->Destroy();
	mirrorChunkToRemove->Destroy();

	int32 indexToRemove = forward ? ActiveChunks.Num() - 1 : 0;
	ActiveChunks.RemoveAt(indexToRemove);
}

void UJGLevelGenerator::SpawnInitialChunks()
{
	if (ChunkClasses.Num() == 0)
		return;

	// Spawn center chunk
	SpawnChunk(true);
		
	// Spawn additional chunks on either side
	for (int32 i = 0; i < NumChunksOnEitherSide; i++)
	{
		SpawnChunk(true);
		SpawnChunk(false);
	}

	// Spawn front and back actors
	SpawnFrontAndBackActors();
}

FChunkData UJGLevelGenerator::GetExtremityChunkData(bool forward)
{
	if (ActiveChunks.Num() > 0)
		return forward ? ActiveChunks[ActiveChunks.Num() - 1] : ActiveChunks[0];
	else
		return FChunkData();
}

FBox UJGLevelGenerator::GetChunkRangeBounds(int32 centerChunkIndex, int32 bufferSize) const
{
	if (ActiveChunks.Num() == 0)
	{
		return FBox(ForceInit);
	}

	FBox totalBounds(ForceInit);
	bool foundAnyChunk = false;

	// Calculate the range of chunk indices we need
	int32 startIndex = centerChunkIndex - bufferSize;
	int32 endIndex = centerChunkIndex + bufferSize;

	// Find chunks within our desired range
	for (const FChunkData& chunkData : ActiveChunks)
	{
		if (chunkData.IsValid())
		{
			int32 chunkLogicalIndex = chunkData.ChunkActor->ChunkLogicalIndex;
			
			// Check if this chunk is in our desired range
			if (chunkLogicalIndex >= startIndex && chunkLogicalIndex <= endIndex)
			{
				FVector chunkLocation = chunkData.ChunkActor->GetActorLocation();
				FVector chunkExtent = chunkData.ActorExtents;
				
				FBox chunkBounds = FBox(chunkLocation - chunkExtent, chunkLocation + chunkExtent);
				
				if (foundAnyChunk)
				{
					totalBounds += chunkBounds;
				}
				else
				{
					totalBounds = chunkBounds;
					foundAnyChunk = true;
				}
			}
		}
	}

	// If we didn't find any chunks in range, estimate bounds based on existing chunks
	if (!foundAnyChunk && ActiveChunks.Num() > 0)
	{
		// Use the first valid chunk as reference for dimensions
		const FChunkData* refChunk = nullptr;
		for (const FChunkData& chunkData : ActiveChunks)
		{
			if (chunkData.IsValid())
			{
				refChunk = &chunkData;
				break;
			}
		}

		if (refChunk)
		{
			FVector chunkExtent = refChunk->ActorExtents;
			float chunkWidth = chunkExtent.X * 2.0f;
			
			// Estimate positions based on chunk spacing
			FVector startLocation = FVector(startIndex * chunkWidth, 0, 0);
			FVector endLocation = FVector(endIndex * chunkWidth, 0, 0);
			
			totalBounds = FBox(
				FVector(startLocation.X - chunkExtent.X, -chunkExtent.Y, -chunkExtent.Z),
				FVector(endLocation.X + chunkExtent.X, chunkExtent.Y, chunkExtent.Z)
			);
		}
	}

	return totalBounds;
}

void UJGLevelGenerator::SpawnFrontAndBackActors()
{
	UWorld* world = GetWorld();
	if (!IsValid(world))
	{
		UE_LOG(LogTemp, Warning, TEXT("JGLevelGenerator: World not found for actor spawning"));
		return;
	}

	// Find player start location
	APlayerStart* playerStart = Cast<APlayerStart>(UGameplayStatics::GetActorOfClass(world, APlayerStart::StaticClass()));
	FVector spawnLocation = FVector::ZeroVector;
	
	if (IsValid(playerStart))
	{
		spawnLocation = playerStart->GetActorLocation();
		UE_LOG(LogTemp, Log, TEXT("JGLevelGenerator: Found PlayerStart at location: %s"), *spawnLocation.ToString());
	}
	else
	{
		// Fallback to origin if no player start found
		UE_LOG(LogTemp, Warning, TEXT("JGLevelGenerator: No PlayerStart found, using origin for actor spawning"));
		spawnLocation = FVector::ZeroVector;
	}

	// Spawn front actor (200m ahead in X direction)
	if (FrontNPCClass)
	{
		FVector frontLocation = spawnLocation + FVector(200.0f, 0.0f, 0.0f);
		FRotator frontRotation = FVector::ForwardVector.Rotation();
		FTransform frontTransform(frontRotation, frontLocation);

		FrontActor = world->SpawnActorDeferred<AJGNPC>(FrontNPCClass, frontTransform);
		FrontActor->MovementDirection = FVector::ForwardVector; // Set movement direction to forward
		FrontActor->FinishSpawning(frontTransform);
		
		if (IsValid(FrontActor))
		{
			UE_LOG(LogTemp, Log, TEXT("JGLevelGenerator: Spawned front actor at location: %s"), *frontLocation.ToString());
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("JGLevelGenerator: Failed to spawn front actor"));
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("JGLevelGenerator: No FrontNPCClass specified"));
	}

	// Spawn back actor (200m behind in X direction)
	if (BackNPCClass)
	{
		FVector backLocation = spawnLocation + FVector(-200.0f, 0.0f, 0.0f);
		FRotator backRotation = FVector::BackwardVector.Rotation();

		FTransform backTransform(backRotation, backLocation);
		BackActor = world->SpawnActorDeferred<AJGNPC>(BackNPCClass, backTransform, nullptr, nullptr, ESpawnActorCollisionHandlingMethod::AlwaysSpawn);
		BackActor->MovementDirection = FVector::BackwardVector; // Set movement direction to backward
		BackActor->FinishSpawning(backTransform);
		
		if (IsValid(BackActor))
		{
			UE_LOG(LogTemp, Log, TEXT("JGLevelGenerator: Spawned back actor at location: %s"), *backLocation.ToString());
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("JGLevelGenerator: Failed to spawn back actor"));
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("JGLevelGenerator: No BackNPCClass specified"));
	}
}