// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "JGChunk.h"
#include "JGLevelGenerator.generated.h"

class AJGNPC;
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnPlayerEnteredChunk, int32, NewChunkIndex, int32, PreviousChunkIndex);

USTRUCT(BlueprintType)
struct FChunkData
{
	GENERATED_BODY()

	UPROPERTY()
	AJGChunk* ChunkActor;

	UPROPERTY()
	AJGChunk* MirrorChunkActor;

	// The extents of the chunk actor
	FVector ActorExtents;

	FChunkData()
		: ChunkActor(nullptr), MirrorChunkActor(nullptr), ActorExtents(FVector::ZeroVector)
	{
	}

	FChunkData(AJGChunk* chunkActor, AJGChunk* mirrorChunk, const FVector& actorExtents)
		: ChunkActor(chunkActor), MirrorChunkActor(mirrorChunk) , ActorExtents(actorExtents)
	{
	}

	bool IsValid() const
    {
        return ChunkActor != nullptr;
    }
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class JAMBEGRANDE_API UJGLevelGenerator : public UActorComponent
{
	GENERATED_BODY()

public:
	UJGLevelGenerator();

	// The number of chunks to keep loaded at once (must be odd number)
	UPROPERTY(EditAnywhere, Category = "Level Generation", meta = (ClampMin = "1"))
	int32 NumChunksOnEitherSide;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Level Generation")
	TArray<TSubclassOf<AJGChunk>> ChunkClasses;

	// Offset applied on Y axis when spawning the mirror chunk
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Level Generation")
	float MirrorYOffset;

	// Actor class to spawn 200m in front of player spawn
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Level Generation")
	TSubclassOf<AJGNPC> FrontNPCClass;

	// Actor class to spawn 200m behind player spawn
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Level Generation")
	TSubclassOf<AJGNPC> BackNPCClass;

protected:
	virtual void BeginPlay() override;

	// Array of currently active chunks, ordered by their logical index
	UPROPERTY()
	TArray<FChunkData> ActiveChunks;

	// The current center chunk's logical index
	UPROPERTY()
	int32 PlayerCurrentChunkIndex;

	// References to the spawned front and back actors
	UPROPERTY()
	AJGNPC* FrontActor;

	UPROPERTY()
	AJGNPC* BackActor;

public:
	// Called when a player enters a new chunk
	UFUNCTION()
	void OnPlayerEnteredChunk(int32 newChunkIndex, int32 previousChunkIndex);

	// Delegate that broadcasts when player enters a new chunk
	UPROPERTY(BlueprintAssignable, Category = "Level Generation")
	FOnPlayerEnteredChunk OnPlayerEnteredChunkDelegate;

	// Get bounds for a range of chunks (for nav mesh generation)
	UFUNCTION(BlueprintCallable, Category = "Level Generation")
	FBox GetChunkRangeBounds(int32 centerChunkIndex, int32 bufferSize) const;

	// Get the active chunks array (read-only access)
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Level Generation")
	const TArray<FChunkData>& GetActiveChunks() const { return ActiveChunks; }

private:
	void SpawnChunk(bool foward);
	void DespawnExtremityChunk(bool forward);
	void SpawnInitialChunks();
	void SpawnFrontAndBackActors();
	
	FChunkData GetExtremityChunkData(bool foward);
};
