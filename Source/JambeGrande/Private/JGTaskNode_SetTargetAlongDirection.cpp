#include "JGTaskNode_SetTargetAlongDirection.h"
#include "AIController.h"
#include "NavigationSystem.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"

UJGTaskNode_SetTargetAlongDirection::UJGTaskNode_SetTargetAlongDirection()
{
	NodeName = "Set Target Along Direction";
	StepDistance = 600.0f;
	MaxSearchRadius = 300.0f;
	NormalWalkSpeed = 200.0f;

	bNotifyTick = false;
}

EBTNodeResult::Type UJGTaskNode_SetTargetAlongDirection::ExecuteTask(UBehaviorTreeComponent& ownerComp, uint8* nodeMemory)
{
	UBlackboardComponent* bb = ownerComp.GetBlackboardComponent();
	if (!IsValid(bb))
	{
		return EBTNodeResult::Failed;
	}

	AAIController* ai = ownerComp.GetAIOwner();
	APawn* npc = ai ? ai->GetPawn() : nullptr;
	if (!IsValid(npc))
	{
		return EBTNodeResult::Failed;
	}

	FVector dir = bb->GetValueAsVector(DirectionKey.SelectedKeyName);
	FVector moveDir = dir.GetSafeNormal();
	if (moveDir.IsNearlyZero())
	{
		moveDir = FVector::ForwardVector;
	}

	const FVector npcLoc = npc->GetActorLocation();
	const FVector desired = npcLoc + moveDir * StepDistance;

	UWorld* world = npc->GetWorld();
	UNavigationSystemV1* nav = world ? FNavigationSystem::GetCurrent<UNavigationSystemV1>(world) : nullptr;

	FVector out = desired;
	if (IsValid(nav))
	{
		FNavLocation navLoc;
		if (nav->ProjectPointToNavigation(desired, navLoc, FVector(MaxSearchRadius)))
		{
			out = navLoc.Location;
		}
	}

	bb->SetValueAsVector(TargetLocationKey.SelectedKeyName, out);

	if (ACharacter* character = Cast<ACharacter>(npc))
	{
		if (UCharacterMovementComponent* move = character->GetCharacterMovement())
		{
			if (move->MaxWalkSpeed != NormalWalkSpeed)
			{
				move->MaxWalkSpeed = NormalWalkSpeed;
			}
		}
	}

	DrawDebugSphere(world, out, 20.0f, 12, FColor::Green, false, 5.0f);

	return EBTNodeResult::Succeeded;
}

FString UJGTaskNode_SetTargetAlongDirection::GetStaticDescription() const
{
	return FString::Printf(TEXT("Set Target Along Direction\nDirection Key: %s\nTarget Location Key: %s\nStep Distance: %.1f\nMax Search Radius: %.1f\nNormal Walk Speed: %.1f"),
		*DirectionKey.SelectedKeyName.ToString(),
		*TargetLocationKey.SelectedKeyName.ToString(),
		StepDistance,
		MaxSearchRadius,
		NormalWalkSpeed);
}


