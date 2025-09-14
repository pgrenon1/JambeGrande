// Fill out your copyright notice in the Description page of Project Settings.

#include "JGTaskNode_FindCutOffLocation.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "AIController.h"
#include "NavigationSystem.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"

UJGTaskNode_FindCutOffLocation::UJGTaskNode_FindCutOffLocation()
{
	NodeName = "Find Cutoff Location";
	
	// Default values
	MaxSearchRadius = 200.0f;
	AheadDistance = 300.0f; // cm
	MaxSpeedBonus = 300.0f; // cm/s (added to normal)
	CachedNormalWalkSpeed = 0.0f;
	
	// Make sure we can abort
	bNotifyTick = false;
	bNotifyTaskFinished = true;
}

EBTNodeResult::Type UJGTaskNode_FindCutOffLocation::ExecuteTask(UBehaviorTreeComponent& ownerComp, uint8* nodeMemory)
{
	AAIController* aiController = ownerComp.GetAIOwner();
	if (!IsValid(aiController))
	{
		UE_LOG(LogTemp, Warning, TEXT("JGTaskNode_FindCutoffLocation: No valid AI controller found"));
		return EBTNodeResult::Failed;
	}

	APawn* controlledPawn = aiController->GetPawn();
	if (!IsValid(controlledPawn))
	{
		UE_LOG(LogTemp, Warning, TEXT("JGTaskNode_FindCutoffLocation: No valid pawn found"));
		return EBTNodeResult::Failed;
	}

	UWorld* world = controlledPawn->GetWorld();
	if (!IsValid(world))
	{
		UE_LOG(LogTemp, Warning, TEXT("JGTaskNode_FindCutoffLocation: No valid world found"));
		return EBTNodeResult::Failed;
	}

	UNavigationSystemV1* navSystem = FNavigationSystem::GetCurrent<UNavigationSystemV1>(world);
	if (!IsValid(navSystem))
	{
		UE_LOG(LogTemp, Warning, TEXT("JGTaskNode_FindCutoffLocation: No navigation system found"));
		return EBTNodeResult::Failed;
	}

	UBlackboardComponent* blackboardComp = ownerComp.GetBlackboardComponent();
	if (!IsValid(blackboardComp))
	{
		UE_LOG(LogTemp, Warning, TEXT("JGTaskNode_FindCutoffLocation: No blackboard component found"));
		return EBTNodeResult::Failed;
	}

	// Get direction from blackboard
	FVector direction = blackboardComp->GetValueAsVector(DirectionKey.SelectedKeyName);
	
	// Validate direction input
	if (direction.IsNearlyZero())
	{
		UE_LOG(LogTemp, Warning, TEXT("JGTaskNode_FindCutoffLocation: Direction vector is zero, using default backward direction"));
		direction = FVector(-1.0f, 0.0f, 0.0f);
	}

	// Get current pawn location
	FVector currentLocation = controlledPawn->GetActorLocation();
	FVector normalizedDirection = direction.GetSafeNormal();

	// Player context
	APawn* playerPawn = GetWorld() && GetWorld()->GetFirstPlayerController() ? GetWorld()->GetFirstPlayerController()->GetPawn() : nullptr;
	if (!IsValid(playerPawn))
	{
		UE_LOG(LogTemp, Warning, TEXT("JGTaskNode_FindCutoffLocation: Player pawn not found"));
		return EBTNodeResult::Failed;
	}

	FVector playerLocation = playerPawn->GetActorLocation();
	FVector playerVelocity = playerPawn->GetVelocity();

	// Movement component and cache/reset normal speed
	ACharacter* npcCharacter = Cast<ACharacter>(controlledPawn);
	UCharacterMovementComponent* moveComp = npcCharacter ? npcCharacter->GetCharacterMovement() : nullptr;
	if (moveComp)
	{
		if (CachedNormalWalkSpeed <= KINDA_SMALL_NUMBER)
		{
			CachedNormalWalkSpeed = moveComp->MaxWalkSpeed;
		}
		// Always reset to normal at the start of this task execution
		moveComp->MaxWalkSpeed = CachedNormalWalkSpeed;
	}

	// Compute cutoff target ahead of player along the desired direction
	FVector targetLocation;
	bool foundValidPoint = FindCutoffLocation(navSystem, currentLocation, normalizedDirection,
		playerLocation,
		playerVelocity,
		AheadDistance,
		MaxSearchRadius,
		targetLocation);

	if (foundValidPoint)
	{
		// Apply a temporary speed boost to ensure we can reach the cutoff
		float boostedSpeed = CachedNormalWalkSpeed + MaxSpeedBonus;
		if (moveComp && moveComp->MaxWalkSpeed < boostedSpeed)
		{
			moveComp->MaxWalkSpeed = boostedSpeed;
		}

		// Store the target location in the blackboard
		blackboardComp->SetValueAsVector(TargetLocationKey.SelectedKeyName, targetLocation);
		UE_LOG(LogTemp, Log, TEXT("JGTaskNode_FindCutoffLocation: Cutoff target: %s (ahead=%.1f)"), *targetLocation.ToString(), AheadDistance);
		DrawDebugSphere(world, targetLocation, 20.0f, 12, FColor::Green, false, 5.0f);
		
		return EBTNodeResult::Succeeded;
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("JGTaskNode_FindCutoffLocation: Could not find cutoff point"));
		return EBTNodeResult::Failed;
	}
}

FString UJGTaskNode_FindCutOffLocation::GetStaticDescription() const
{
	return FString::Printf(TEXT("Find Cutoff Location\nDirection Key: %s\nTarget Location Key: %s\nMax Search Radius: %.1f\nAhead Distance: %.1f\nMax Speed Bonus: %.1f"),
		*DirectionKey.SelectedKeyName.ToString(),
		*TargetLocationKey.SelectedKeyName.ToString(),
		MaxSearchRadius,
		AheadDistance,
		MaxSpeedBonus);
}

bool UJGTaskNode_FindCutOffLocation::FindCutoffLocation(UNavigationSystemV1* navSystem,
	const FVector& npcLocation,
	const FVector& direction,
	const FVector& playerLocation,
	const FVector& playerVelocity,
	float aheadDistance,
	float maxSearchRadius,
	FVector& outLocation) const
{
	if (!IsValid(navSystem))
	{
		return false;
	}

	// Align strictly with the sidewalk direction and operate in 2D
	FVector moveDir = direction.GetSafeNormal();
	if (moveDir.IsNearlyZero())
	{
		moveDir = FVector::ForwardVector;
	}

	const FVector2D moveDir2D(moveDir.X, moveDir.Y);
	const FVector2D playerLoc2D(playerLocation.X, playerLocation.Y);
	const FVector2D playerVel2D(playerVelocity.X, playerVelocity.Y);

	// Lead distance scales with player's forward speed along the sidewalk
	const float playerForwardSpeed = FVector2D::DotProduct(playerVel2D, moveDir2D);
	const float leadFromSpeed = FMath::Clamp(playerForwardSpeed * 0.5f, 0.0f, aheadDistance);
	const float desiredLead = aheadDistance + leadFromSpeed;

	// Ideal point: ahead of the player along the lane (no lateral offset)
	FVector2D ideal2D = playerLoc2D + moveDir2D * desiredLead;
	FVector ideal3D(ideal2D.X, ideal2D.Y, playerLocation.Z);

	UWorld* world = navSystem->GetWorld();

	FNavLocation navLocation;
	if (navSystem->ProjectPointToNavigation(ideal3D, navLocation, FVector(maxSearchRadius)))
	{
		outLocation = navLocation.Location;
		if (IsValid(world))
		{
			DrawDebugLine(world, playerLocation, ideal3D, FColor::Cyan, false, 2.0f, 0, 1.5f);
			DrawDebugSphere(world, outLocation, 22.0f, 12, FColor::Green, false, 2.0f);
		}
		return true;
	}

	// Longitudinal search only (keeps to the same lane)
	const int32 numSteps = 8;
	const float step = FMath::Max(25.0f, maxSearchRadius / (float)numSteps);
	for (int32 i = 1; i <= numSteps; ++i)
	{
		const float delta = step * (float)i;

		// Step backward from ideal
		FVector2D testBack2D = ideal2D - moveDir2D * delta;
		FVector testBack3D(testBack2D.X, testBack2D.Y, playerLocation.Z);
		if (navSystem->ProjectPointToNavigation(testBack3D, navLocation, FVector(maxSearchRadius)))
		{
			outLocation = navLocation.Location;
			if (IsValid(world))
			{
				DrawDebugSphere(world, outLocation, 20.0f, 12, FColor::Green, false, 2.0f);
			}
			return true;
		}

		// Step further ahead from ideal
		FVector2D testAhead2D = ideal2D + moveDir2D * delta;
		FVector testAhead3D(testAhead2D.X, testAhead2D.Y, playerLocation.Z);
		if (navSystem->ProjectPointToNavigation(testAhead3D, navLocation, FVector(maxSearchRadius)))
		{
			outLocation = navLocation.Location;
			if (IsValid(world))
			{
				DrawDebugSphere(world, outLocation, 20.0f, 12, FColor::Green, false, 2.0f);
			}
			return true;
		}
	}

	// Fallback: ahead of the NPC along the lane
	FVector2D npcAhead2D = FVector2D(npcLocation.X, npcLocation.Y) + moveDir2D * aheadDistance;
	FVector npcAhead3D(npcAhead2D.X, npcAhead2D.Y, npcLocation.Z);
	if (navSystem->ProjectPointToNavigation(npcAhead3D, navLocation, FVector(maxSearchRadius)))
	{
		outLocation = navLocation.Location;
		if (IsValid(world))
		{
			DrawDebugSphere(world, outLocation, 22.0f, 12, FColor::Yellow, false, 2.0f);
		}
		return true;
	}

	if (IsValid(world))
	{
		DrawDebugSphere(world, ideal3D, 12.0f, 10, FColor::Red, false, 2.0f);
	}

	return false;
}