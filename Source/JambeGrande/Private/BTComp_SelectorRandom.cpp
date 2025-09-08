
#include "BTComp_SelectorRandom.h"
#include "BehaviorTree/BehaviorTreeTypes.h"
#include "BehaviorTree/BehaviorTreeComponent.h"

UBTComp_SelectorRandom::UBTComp_SelectorRandom()
{
	NodeName = TEXT("Selector (Weighted Random)");
}

int32 UBTComp_SelectorRandom::GetNextChildHandler(FBehaviorTreeSearchData& searchData, int32 prevChild, EBTNodeResult::Type lastResult) const
{
	if (Children.Num() == 0)
	{
		return BTSpecialChild::ReturnToParent;
	}

	if (prevChild != BTSpecialChild::NotInitialized)
	{
		return BTSpecialChild::ReturnToParent;
	}

	TArray<int32> allowedIndices;
	allowedIndices.Reserve(Children.Num());
	for (int32 childIndex = 0; childIndex < Children.Num(); ++childIndex)
	{
		if (DoDecoratorsAllowExecution(searchData.OwnerComp, searchData.OwnerComp.GetActiveInstanceIdx(), childIndex))
		{
			allowedIndices.Add(childIndex);
		}
	}

	if (allowedIndices.Num() == 0)
	{
		return BTSpecialChild::ReturnToParent;
	}

	float totalWeight = 0.0f;
	TArray<float> effectiveWeights;
	effectiveWeights.Reserve(allowedIndices.Num());
	for (int32 i = 0; i < allowedIndices.Num(); ++i)
	{
		const int32 childIndex = allowedIndices[i];
		const float weight = Weights.IsValidIndex(childIndex) ? FMath::Max(0.0f, Weights[childIndex]) : 1.0f;
		effectiveWeights.Add(weight);
		totalWeight += weight;
	}

	if (totalWeight <= KINDA_SMALL_NUMBER)
	{
		const int32 pick = allowedIndices[FMath::RandRange(0, allowedIndices.Num() - 1)];
		return pick;
	}

	const float roll = FMath::FRand() * totalWeight;
	float cumulative = 0.0f;
	for (int32 i = 0; i < allowedIndices.Num(); ++i)
	{
		cumulative += effectiveWeights[i];
		if (roll <= cumulative)
		{
			return allowedIndices[i];
		}
	}

	return allowedIndices.Last();
}

FString UBTComp_SelectorRandom::GetStaticDescription() const
{
	FString desc = Super::GetStaticDescription();
	if (Children.Num() > 0)
	{
		TArray<FString> weightStrings;
		weightStrings.Reserve(Children.Num());
		for (int32 i = 0; i < Children.Num(); ++i)
		{
			const float w = Weights.IsValidIndex(i) ? Weights[i] : 1.0f;
			weightStrings.Add(FString::SanitizeFloat(w));
		}
		desc += TEXT("\nWeights: ") + FString::Join(weightStrings, TEXT(", "));
	}
	return desc;
}
