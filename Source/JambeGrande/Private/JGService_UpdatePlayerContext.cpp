#include "JGService_UpdatePlayerContext.h"
#include "AIController.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType_Float.h"
#include "GameFramework/Pawn.h"

UJGService_UpdatePlayerContext::UJGService_UpdatePlayerContext()
{
	NodeName = "Update Player Context";
	Interval = 0.2f;
	RandomDeviation = 0.0f;
	bNotifyCeaseRelevant = false;
}

void UJGService_UpdatePlayerContext::UpdatePlayerContext(UBehaviorTreeComponent& ownerComp)
{
	UBlackboardComponent* bb = ownerComp.GetBlackboardComponent();
	if (!IsValid(bb))
	{
		return;
	}

	AAIController* ai = ownerComp.GetAIOwner();
	APawn* npc = ai ? ai->GetPawn() : nullptr;
	APawn* player = ownerComp.GetWorld() && ownerComp.GetWorld()->GetFirstPlayerController()
		                ? ownerComp.GetWorld()->GetFirstPlayerController()->GetPawn() : nullptr;

	if (!IsValid(npc) || !IsValid(player))
	{
		return;
	}

	// Read movement direction from blackboard and normalize to 2D
	FVector rawDir = bb->GetValueAsVector(DirectionKey.SelectedKeyName);
	FVector moveDir = rawDir.GetSafeNormal();
	if (moveDir.IsNearlyZero())
	{
		moveDir = FVector::ForwardVector;
	}

	const FVector2D moveDir2D(moveDir.X, moveDir.Y);
	const FVector2D npc2D(npc->GetActorLocation().X, npc->GetActorLocation().Y);
	const FVector2D player2D(player->GetActorLocation().X, player->GetActorLocation().Y);

	// Project delta onto direction to get longitudinal separation (signed)
	const FVector2D delta2D = player2D - npc2D;
	const float signedLongitudinal = FVector2D::DotProduct(delta2D, moveDir2D);

	bb->SetValueAsFloat(PlayerLongitudinalOffsetKey.SelectedKeyName, signedLongitudinal);

	// Player facing dot with DirectionKey (2D)
	FVector playerForward = player->GetActorForwardVector();
	FVector2D playerForward2D(playerForward.X, playerForward.Y);
	FVector2D normMoveDir2D = moveDir2D.GetSafeNormal();
	FVector2D normForward2D = playerForward2D.GetSafeNormal();
	const float facingDot = FVector2D::DotProduct(normForward2D, normMoveDir2D);
	bb->SetValueAsFloat(PlayerFacingDotKey.SelectedKeyName, facingDot);
}

void UJGService_UpdatePlayerContext::TickNode(UBehaviorTreeComponent& ownerComp, uint8* nodeMemory, float deltaSeconds)
{
	Super::TickNode(ownerComp, nodeMemory, deltaSeconds);

	UpdatePlayerContext(ownerComp);
}

void UJGService_UpdatePlayerContext::OnSearchStart(FBehaviorTreeSearchData& searchData)
{
	Super::OnSearchStart(searchData);

	UpdatePlayerContext(searchData.OwnerComp);
}
