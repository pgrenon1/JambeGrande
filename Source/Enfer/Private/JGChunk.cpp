// Fill out your copyright notice in the Description page of Project Settings.


#include "Public/JGChunk.h"
#include "Components/StaticMeshComponent.h"
#include "Components/PrimitiveComponent.h"
#include "Engine/World.h"
#include "AssetRegistry/AssetData.h"

// Sets default values
AJGChunk::AJGChunk()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	// set up the empty root component
	USceneComponent* root = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));
	RootComponent = root;
	
	// Setup the BuildingChildActor
	BuildingChildActor = CreateDefaultSubobject<UChildActorComponent>(TEXT("BuildingChildActor"));
	BuildingChildActor->SetupAttachment(RootComponent);

	WallBoxCollision = CreateDefaultSubobject<UBoxComponent>(TEXT("WallBoxCollision"));
	WallBoxCollision->SetupAttachment(RootComponent);

	FloorParent = CreateDefaultSubobject<USceneComponent>(TEXT("FloorParent"));
	FloorParent->SetupAttachment(RootComponent);

	// Create and setup the box component
	TriggerBoxComponent = CreateDefaultSubobject<UBoxComponent>(TEXT("TriggerBoxComponent"));
	TriggerBoxComponent->SetupAttachment(RootComponent);
	
	// Set the box size (adjust as needed)
	TriggerBoxComponent->SetBoxExtent(FVector(50.0f, 50.0f, 50.0f));
	TriggerBoxComponent->SetCollisionProfileName(TEXT("Trigger"));
	TriggerBoxComponent->SetGenerateOverlapEvents(true);
}

void AJGChunk::SetIndex(int32 index)
{
	ChunkLogicalIndex = index;
}

void AJGChunk::GetBuildingBounds(FVector& location, FVector& extent) const
{
	UE_LOG(LogTemp, Log, TEXT("=== Starting GetBuildingBounds for chunk %d ==="), ChunkLogicalIndex);
	
	TSubclassOf<AActor> buildingActorClass = BuildingChildActor->GetChildActorClass();
	if (!IsValid(buildingActorClass))
	{
		UE_LOG(LogTemp, Warning, TEXT("No valid building actor class found in BuildingChildActor"));
		return;
	}

	UE_LOG(LogTemp, Log, TEXT("Building actor class: %s"), *buildingActorClass->GetName());

	// Try to get world context, fallback to editor world if needed
	UWorld* world = GetWorld();
	if (!IsValid(world))
	{
		// We're likely being called from an asset action utility (CDO context)
		// Try to get the editor world
#if WITH_EDITOR
		if (GEditor && GEditor->GetEditorWorldContext().World())
		{
			world = GEditor->GetEditorWorldContext().World();
			UE_LOG(LogTemp, Log, TEXT("Using editor world for bounds calculation"));
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("Could not get valid world context for bounds calculation"));
			return;
		}
#endif
	}

	// Spawn the actor temporarily at origin
	FActorSpawnParameters spawnParams;
	spawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	spawnParams.bDeferConstruction = false;
	
	AActor* tempActor = world->SpawnActor<AActor>(buildingActorClass, FVector::ZeroVector, FRotator::ZeroRotator, spawnParams);
	if (!IsValid(tempActor))
	{
		UE_LOG(LogTemp, Warning, TEXT("Could not spawn temporary building actor"));
		return;
	}

	// Get all primitive components and manually calculate bounds excluding unwanted ones
	TArray<UPrimitiveComponent*> primitiveComponents;
	tempActor->GetComponents<UPrimitiveComponent>(primitiveComponents);
	
	FBox combinedBox(ForceInit);
	bool foundValidComponent = false;
	int32 processedCount = 0;
	int32 skippedCount = 0;
	
	for (UPrimitiveComponent* component : primitiveComponents)
	{
		if (!IsValid(component))
		{
			continue;
		}
		
		// Skip components with no collision
		if (component->GetCollisionEnabled() == ECollisionEnabled::NoCollision)
		{
			skippedCount++;
			UE_LOG(LogTemp, Log, TEXT("Skipping no-collision component: %s"), *component->GetName());
			continue;
		}
		
		// Skip stairs and steps components
		FString componentName = component->GetName();
		if (componentName.Contains(TEXT("stairs"), ESearchCase::IgnoreCase) || 
			componentName.Contains(TEXT("steps"), ESearchCase::IgnoreCase))
		{
			skippedCount++;
			UE_LOG(LogTemp, Log, TEXT("Skipping stairs/steps component: %s"), *componentName);
			continue;
		}
		
		// Get component bounds
		FBoxSphereBounds componentBounds = component->Bounds;
		FBox componentBox = FBox::BuildAABB(componentBounds.Origin, componentBounds.BoxExtent);
		
		UE_LOG(LogTemp, Log, TEXT("Processing component: %s - Origin: %s, Extent: %s"), 
			*componentName,
			*componentBounds.Origin.ToString(), 
			*componentBounds.BoxExtent.ToString());
		
		if (!foundValidComponent)
		{
			combinedBox = componentBox;
			foundValidComponent = true;
			UE_LOG(LogTemp, Log, TEXT("  Set as initial combined box - Min: %s, Max: %s"), 
				*combinedBox.Min.ToString(), 
				*combinedBox.Max.ToString());
		}
		else
		{
			FBox previousBox = combinedBox;
			combinedBox += componentBox;
			UE_LOG(LogTemp, Log, TEXT("  Added to combined box - Previous: [%s, %s], New: [%s, %s]"), 
				*previousBox.Min.ToString(), *previousBox.Max.ToString(),
				*combinedBox.Min.ToString(), *combinedBox.Max.ToString());
		}
		
		processedCount++;
	}
	
	// Clean up the temporary actor
	tempActor->Destroy();
	
	UE_LOG(LogTemp, Log, TEXT("Component processing summary:"));
	UE_LOG(LogTemp, Log, TEXT("  - Processed: %d"), processedCount);
	UE_LOG(LogTemp, Log, TEXT("  - Skipped: %d"), skippedCount);
	
	if (foundValidComponent)
	{
		location = combinedBox.GetCenter();
		extent = combinedBox.GetExtent();
		UE_LOG(LogTemp, Log, TEXT("Final building bounds - Location: %s, Extent: %s"), 
			*location.ToString(), 
			*extent.ToString());
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("No valid components found for building bounds calculation"));
	}
	
	UE_LOG(LogTemp, Log, TEXT("=== Finished GetBuildingBounds ==="));
}

void AJGChunk::SetupFloor()
{
    if (!IsValid(FloorParent))
    	return;
	
	FVector floorScale = FVector(WallBoxCollision->GetScaledBoxExtent().X / 100.0f * 2.0f, 5.0f, 1.0f);
	
	FVector floorLocation = FVector(floorScale.X / 2.0f * 100.0f, 250.0f, -50.0f);
	
	FloorParent->SetRelativeLocation(floorLocation);
    FloorParent->SetRelativeScale3D(floorScale);
}

void AJGChunk::SetupTriggerBox()
{
	// Check if we have valid components to work with
	if (!IsValid(TriggerBoxComponent) || !IsValid(WallBoxCollision))
	{
		UE_LOG(LogTemp, Warning, TEXT("TriggerBoxComponent or WallBoxCollision is not valid!"));
		return;
	}
	
	// Perform the component modifications
	FVector newLocation = WallBoxCollision->GetComponentLocation();
	newLocation.Y = 0.0f; // Ensure Y is zero for the trigger box
	TriggerBoxComponent->SetRelativeLocation(newLocation);

	auto wallBoxExtent = WallBoxCollision->GetScaledBoxExtent();
	wallBoxExtent.Y = 500.0f; // Set Y extent for the trigger box

	TriggerBoxComponent->SetBoxExtent(wallBoxExtent);
}

void AJGChunk::SetupWallBoxCollision()
{
	if (!IsValid(WallBoxCollision))
	{
		UE_LOG(LogTemp, Warning, TEXT("WallBoxCollision is not valid!"));
		return;
	}
	
	// Get the building bounds from the child actor
	FVector boxExtent;
	FVector boxLocation;
	GetBuildingBounds(boxLocation, boxExtent);
	
	// Set the collision box to match the wall bounds
	WallBoxCollision->SetBoxExtent(boxExtent);
	WallBoxCollision->SetWorldLocation(boxLocation);
	
	// Configure collision settings
	WallBoxCollision->SetCollisionProfileName(TEXT("BlockAll"));
	WallBoxCollision->SetGenerateOverlapEvents(true);
}