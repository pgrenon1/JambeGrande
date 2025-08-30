#include "JGTaskNode_TeleportAhead.h"
#include "AIController.h"
#include "NavigationSystem.h"
#include "Kismet/GameplayStatics.h"

UJGTaskNode_TeleportAhead::UJGTaskNode_TeleportAhead()
{
	NodeName = "Teleport Ahead";
	LaneY = 0.0f;
	AheadDistance = 100.0f;
	ProjectToNavMesh = true;
	MaxSearchRadius = 300.0f;

	bNotifyTick = false;
}

EBTNodeResult::Type UJGTaskNode_TeleportAhead::ExecuteTask(UBehaviorTreeComponent& ownerComp, uint8* nodeMemory)
{
	AAIController* ai = ownerComp.GetAIOwner();
	APawn* npc = ai ? ai->GetPawn() : nullptr;
	if (!IsValid(npc))
	{
		return EBTNodeResult::Failed;
	}

	APawn* player = UGameplayStatics::GetPlayerPawn(npc, 0);
	if (!IsValid(player))
	{
		return EBTNodeResult::Failed;
	}

	UBlackboardComponent* bb = ownerComp.GetBlackboardComponent();
	if (!IsValid(bb))
	{
		return EBTNodeResult::Failed;
	}

	// Direction considered as "ahead"
	FVector rawDir = bb->GetValueAsVector(DirectionKey.SelectedKeyName);
	FVector moveDir = rawDir.GetSafeNormal();
	if (moveDir.IsNearlyZero())
	{
		moveDir = FVector::ForwardVector;
	}
	FVector2D moveDir2D(moveDir.X, moveDir.Y);
	moveDir2D.Normalize();

	// Compute target ahead of player at fixed distance, clamp Y to lane
	FVector playerLoc = player->GetActorLocation();
	FVector2D player2D(playerLoc.X, playerLoc.Y);
	FVector2D target2D = player2D + moveDir2D * AheadDistance;
	target2D.Y = LaneY;

	FVector target3D(target2D.X, target2D.Y, 0.0f);

	// Optionally project to navmesh
	if (ProjectToNavMesh)
	{
		UWorld* world = npc->GetWorld();
		if (IsValid(world))
		{
			UNavigationSystemV1* nav = FNavigationSystem::GetCurrent<UNavigationSystemV1>(world);
			if (IsValid(nav))
			{
				FNavLocation navLoc;
				if (nav->ProjectPointToNavigation(target3D, navLoc, FVector(MaxSearchRadius)))
				{
					target3D = navLoc.Location;
				}
			}
		}
	}

	target3D += FVector(0.0f, 0.0f, npc->GetDefaultHalfHeight());

	DrawDebugSphere(npc->GetWorld(), target3D, 20.0f, 12, FColor::Green, false, 2.0f);
	
	// Teleport NPC
	npc->SetActorLocation(target3D, false, nullptr, ETeleportType::TeleportPhysics);

	return EBTNodeResult::Succeeded;
}

FString UJGTaskNode_TeleportAhead::GetStaticDescription() const
{
	return FString::Printf(TEXT("Teleport Ahead\nDirection Key: %s\nLaneY: %.1f\nAhead Distance: %.1f\nProject To NavMesh: %s\nMax Search Radius: %.1f"),
		*DirectionKey.SelectedKeyName.ToString(),
		LaneY,
		AheadDistance,
		ProjectToNavMesh ? TEXT("true") : TEXT("false"),
		MaxSearchRadius);
}


