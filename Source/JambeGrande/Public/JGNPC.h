// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "JGNPC.generated.h"

UCLASS()
class JAMBEGRANDE_API AJGNPC : public ACharacter
{
	GENERATED_BODY()

public:
	AJGNPC();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC")
	FVector MovementDirection;
};
