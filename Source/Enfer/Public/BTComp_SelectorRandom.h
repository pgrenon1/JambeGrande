#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTCompositeNode.h"
#include "BTComp_SelectorRandom.generated.h"

UCLASS()
class ENFER_API UBTComp_SelectorRandom : public UBTCompositeNode
{
	GENERATED_BODY()

	public:
	UBTComp_SelectorRandom();

	virtual int32 GetNextChildHandler(FBehaviorTreeSearchData& searchData, int32 prevChild, EBTNodeResult::Type lastResult) const override;
	virtual FString GetStaticDescription() const override;

	protected:
	// Optional per-child weights; defaults to 1.0 for unspecified entries
	UPROPERTY(EditAnywhere, Category = "Selector")
	TArray<float> Weights;
};
