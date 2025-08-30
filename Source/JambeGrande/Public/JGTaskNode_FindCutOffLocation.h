// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "NavigationSystem.h"
#include "JGTaskNode_FindCutOffLocation.generated.h"

/**
 * Behavior tree task node that finds a point on the nav mesh in a specified direction
 * at a specified distance from the controlled character
 */
UCLASS()
class JAMBEGRANDE_API UJGTaskNode_FindCutOffLocation : public UBTTaskNode
{
	GENERATED_BODY()

public:
	UJGTaskNode_FindCutOffLocation();

	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& ownerComp, uint8* nodeMemory) override;
	virtual FString GetStaticDescription() const override;

protected:
	// Blackboard key to read the direction vector from
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Blackboard")
	FBlackboardKeySelector DirectionKey;

	// Blackboard key to store the found target location
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Blackboard")
	FBlackboardKeySelector TargetLocationKey;

	// Maximum search radius if the exact distance point is not valid
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement", meta = (ClampMin = "0.0"))
	float MaxSearchRadius;

	// How far in front of the player the NPC should move along DirectionKey
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Cutoff", meta = (ClampMin = "0.0"))
	float AheadDistance;

	// Maximum additional speed above normal the NPC can use when boosting to cut off
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Cutoff", meta = (ClampMin = "0.0"))
	float MaxSpeedBonus;

private:
	// Cached normal walk speed for temporary boost calculations
	UPROPERTY(Transient)
	float CachedNormalWalkSpeed;

	// Compute a cutoff location ahead of the player, aligned with the movement direction, and project to navmesh
	bool FindCutoffLocation(UNavigationSystemV1* navSystem,
		const FVector& npcLocation,
		const FVector& direction,
		const FVector& playerLocation,
		const FVector& playerVelocity,
		float aheadDistance,
		float maxSearchRadius,
		FVector& outLocation) const;
};
