#include "JGService_UpdateTargetAlongDirection.h"
#include "AIController.h"
#include "NavigationSystem.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"

UJGService_UpdateTargetAlongDirection::UJGService_UpdateTargetAlongDirection()
{
	NodeName = "Update Target Along Direction";
	Interval = 0.25f;
	RandomDeviation = 0.0f;

	StepDistance = 600.0f;
	MaxSearchRadius = 300.0f;
	NormalWalkSpeed = 200.0f;
}

void UJGService_UpdateTargetAlongDirection::TickNode(UBehaviorTreeComponent& ownerComp, uint8* nodeMemory, float deltaSeconds)
{
	Super::TickNode(ownerComp, nodeMemory, deltaSeconds);

	UBlackboardComponent* bb = ownerComp.GetBlackboardComponent();
	if (!IsValid(bb))
	{
		return;
	}

	AAIController* ai = ownerComp.GetAIOwner();
	APawn* npc = ai ? ai->GetPawn() : nullptr;
	if (!IsValid(npc))
	{
		return;
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
}