#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "Animation/AnimMontage.h"
#include "JGTaskNode_PlayWeightedMontage.generated.h"

USTRUCT(BlueprintType)
struct FWeightedMontageEntry
{
	GENERATED_BODY()

	// Montage asset to play (can be null in entries array, but typically use NullWeight for no-montage case)
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	UAnimMontage* Montage = nullptr;

	// Relative weight for random selection (must be >= 0)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (ClampMin = "0.0"))
	float Weight = 1.0f;
};

UCLASS()
class ENFER_API UJGTaskNode_PlayWeightedMontage : public UBTTaskNode
{
	GENERATED_BODY()

public:
	UJGTaskNode_PlayWeightedMontage();

	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& ownerComp, uint8* nodeMemory) override;
	virtual FString GetStaticDescription() const override;

protected:
	// Candidate montages with weights
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Montage")
	TArray<FWeightedMontageEntry> Montages;

	// Play rate for the selected montage
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Montage", meta = (ClampMin = "0.01"))
	float PlayRate;

	// If true, stop all current montages before playing the selected montage
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Montage")
	bool StopExistingMontages;

	// If true, the task will wait until the montage should have finished before completing
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Montage")
	bool WaitForMontageToFinish;

	// Optional slot name to play the montage in (empty = use montage defaults)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Montage")
	FName SlotName;

	FOnMontageEnded MontageEndedDelegate;

private:
	void OnMontageEnded(UAnimMontage* montage, bool bInterrupted, UBehaviorTreeComponent* ownerComp, UAnimInstance* animInstance);
};


