// Fill out your copyright notice in the Description page of Project Settings.


#include "JGNPC.h"
#include "Components/SkeletalMeshComponent.h"


// Sets default values
AJGNPC::AJGNPC()
{
	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}

void AJGNPC::BeginPlay()
{
	Super::BeginPlay();

	ApplyRandomMesh();
}

void AJGNPC::UpdateCastShadow() const
{
	APawn* player = GetWorld() && GetWorld()->GetFirstPlayerController()
		                ? GetWorld()->GetFirstPlayerController()->GetPawn() : nullptr;

	FVector2D moveDir2D = FVector2D(MovementDirection.X, MovementDirection.Y);
	FVector playerForward = player->GetActorForwardVector();
	FVector2D playerForward2D(playerForward.X, playerForward.Y);
	const float facingDot = FVector2D::DotProduct(playerForward2D.GetSafeNormal(), moveDir2D.GetSafeNormal());
	GetMesh()->SetCastShadow(facingDot > -0.8f);
}

void AJGNPC::Tick(float deltaSeconds)
{
	Super::Tick(deltaSeconds);

	// UpdateCastShadow();
}

void AJGNPC::ApplyRandomMesh()
{
	USkeletalMeshComponent* characterMesh = GetMesh();
	if (!IsValid(characterMesh))
	{
		return;
	}

	if (WeightedMeshes.Num() == 0)
	{
		return;
	}

	float totalChance = 0.0f;
	for (const FWeightedSkeletalMesh& entry : WeightedMeshes)
	{
		if (entry.Chance > 0.0f && IsValid(entry.Mesh))
		{
			totalChance += entry.Chance;
		}
	}

	if (totalChance <= 0.0f)
	{
		return;
	}

	const float roll = FMath::FRandRange(0.0f, totalChance);
	float cumulative = 0.0f;
	for (const FWeightedSkeletalMesh& entry : WeightedMeshes)
	{
		if (entry.Chance <= 0.0f || !IsValid(entry.Mesh))
		{
			continue;
		}

		cumulative += entry.Chance;
		if (roll <= cumulative)
		{
			characterMesh->SetSkeletalMesh(entry.Mesh);
			return;
		}
	}

	// Fallback: set to first valid mesh if rounding errors prevented selection
	for (const FWeightedSkeletalMesh& entry : WeightedMeshes)
	{
		if (IsValid(entry.Mesh))
		{
			characterMesh->SetSkeletalMesh(entry.Mesh);
			return;
		}
	}
}
