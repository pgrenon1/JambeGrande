// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/BoxComponent.h"
#include "JGChunk.generated.h"

UCLASS()
class ENFER_API AJGChunk : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	AJGChunk();

	UPROPERTY(EditAnywhere, Category = "Chunk")
	UBoxComponent* TriggerBoxComponent;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Chunk")
	UChildActorComponent* BuildingChildActor;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Chunk")
	UBoxComponent* WallBoxCollision;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Chunk")
	USceneComponent* FloorParent;
	
	void SetIndex(int32 index);
	void GetBuildingBounds(FVector& location, FVector& extent) const;
	void SetupFloor();
	void SetupTriggerBox();
	void SetupWallBoxCollision();

	int32 ChunkLogicalIndex;
};
