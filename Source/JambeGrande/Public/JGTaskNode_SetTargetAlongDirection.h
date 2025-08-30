#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "JGTaskNode_SetTargetAlongDirection.generated.h"

UCLASS()
class JAMBEGRANDE_API UJGTaskNode_SetTargetAlongDirection : public UBTTaskNode
{
	GENERATED_BODY()

public:
	UJGTaskNode_SetTargetAlongDirection();

	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& ownerComp, uint8* nodeMemory) override;
	virtual FString GetStaticDescription() const override;

protected:
	// Direction along which the NPC should move (world-space). Used to place a point ahead.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Blackboard")
	FBlackboardKeySelector DirectionKey;

	// Blackboard key to store the computed target location
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Blackboard")
	FBlackboardKeySelector TargetLocationKey;

	// How far ahead of the NPC to place the target point
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement", meta = (ClampMin = "0.0"))
	float StepDistance;

	// Max radius for navmesh projection
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement", meta = (ClampMin = "0.0"))
	float MaxSearchRadius;

	// Optional: reset NPC's normal walking speed when setting the target
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement", meta = (ClampMin = "0.0"))
	float NormalWalkSpeed;
};


