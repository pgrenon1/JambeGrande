// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "JGNPC.generated.h"

class USkeletalMesh;

USTRUCT(BlueprintType)
struct FWeightedSkeletalMesh
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Mesh")
	USkeletalMesh* Mesh;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Mesh", meta = (ClampMin = "0.0"))
	float Chance;

	FWeightedSkeletalMesh()
		: Mesh(nullptr)
		, Chance(1.0f)
	{
	}
};

UCLASS()
class ENFER_API AJGNPC : public ACharacter
{
	GENERATED_BODY()

public:
	AJGNPC();

	virtual void BeginPlay() override;
	void UpdateCastShadow() const;

	virtual void Tick(float deltaSeconds) override;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC")
	FVector MovementDirection;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Mesh")
	TArray<FWeightedSkeletalMesh> WeightedMeshes;

	UFUNCTION(BlueprintCallable, Category = "NPC|Mesh")
	void ApplyRandomMesh();
};
