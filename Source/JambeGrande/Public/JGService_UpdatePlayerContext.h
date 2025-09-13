#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTService.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "JGService_UpdatePlayerContext.generated.h"

UCLASS()
class JAMBEGRANDE_API UJGService_UpdatePlayerContext : public UBTService
{
	GENERATED_BODY()

public:
	UJGService_UpdatePlayerContext();
	void UpdatePlayerContext(UBehaviorTreeComponent& ownerComp);

protected:
	// Direction along which the NPC is moving (world-space). Used to project player/NPC delta.
	UPROPERTY(EditAnywhere, Category = "Blackboard")
	FBlackboardKeySelector DirectionKey;

	// Optional: if set, will receive the signed longitudinal offset (player relative to NPC along DirectionKey)
	UPROPERTY(EditAnywhere, Category = "Blackboard")
	FBlackboardKeySelector PlayerLongitudinalOffsetKey;

	// Optional: if set, will receive the player's facing dot with DirectionKey (2D)
	UPROPERTY(EditAnywhere, Category = "Blackboard")
	FBlackboardKeySelector PlayerFacingDotKey;

	virtual void TickNode(UBehaviorTreeComponent& ownerComp, uint8* nodeMemory, float deltaSeconds) override;

	virtual void OnSearchStart(FBehaviorTreeSearchData& searchData) override;
};