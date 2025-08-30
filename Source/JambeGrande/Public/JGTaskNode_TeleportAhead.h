#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "JGTaskNode_TeleportAhead.generated.h"

UCLASS()
class JAMBEGRANDE_API UJGTaskNode_TeleportAhead : public UBTTaskNode
{
	GENERATED_BODY()

public:
	UJGTaskNode_TeleportAhead();

	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& ownerComp, uint8* nodeMemory) override;
	virtual FString GetStaticDescription() const override;

protected:
	// Direction considered as ahead (world-space)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Blackboard")
	FBlackboardKeySelector DirectionKey;

	// Desired lane Y coordinate to clamp to when teleporting
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Teleport")
	float LaneY;

	// Distance ahead of the player to place the NPC when teleporting
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Teleport", meta = (ClampMin = "0.0"))
	float AheadDistance;

	// If true, project the teleport point to navmesh (may slightly change Y)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Teleport")
	bool ProjectToNavMesh;

	// Radius for navmesh projection when enabled
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Teleport", meta = (ClampMin = "0.0"))
	float MaxSearchRadius;
};


