#include "JGTaskNode_PlayWeightedMontage.h"
#include "AIController.h"
#include "GameFramework/Character.h"
#include "Animation/AnimInstance.h"

UJGTaskNode_PlayWeightedMontage::UJGTaskNode_PlayWeightedMontage()
{
	NodeName = TEXT("Play Weighted Montage");
	PlayRate = 1.0f;
	StopExistingMontages = false;
	WaitForMontageToFinish = true;
}

EBTNodeResult::Type UJGTaskNode_PlayWeightedMontage::ExecuteTask(UBehaviorTreeComponent& ownerComp, uint8* nodeMemory)
{
	AAIController* ai = ownerComp.GetAIOwner();
	ACharacter* character = ai ? Cast<ACharacter>(ai->GetPawn()) : nullptr;
	if (!IsValid(character))
	{
		return EBTNodeResult::Failed;
	}

	UAnimInstance* anim = character->GetMesh() ? character->GetMesh()->GetAnimInstance() : nullptr;
	if (!IsValid(anim))
	{
		return EBTNodeResult::Failed;
	}

	// Compute total weight across entries
	float total = 0.0f;
	for (const FWeightedMontageEntry& entry : Montages)
	{
		total += FMath::Max(0.0f, entry.Weight);
	}
	if (total <= 0.0f)
	{
		return EBTNodeResult::Succeeded; // nothing to do
	}

	float pick = FMath::FRandRange(0.0f, total);
	UAnimMontage* chosen = nullptr;
	for (const FWeightedMontageEntry& entry : Montages)
	{
		const float w = FMath::Max(0.0f, entry.Weight);
		if (pick <= w)
		{
			chosen = entry.Montage;
			break;
		}
		pick -= w;
	}

	if (!IsValid(chosen))
	{
		return EBTNodeResult::Succeeded; // treat invalid as no-op
	}

	if (StopExistingMontages)
	{
		anim->StopAllMontages(0.1f);
	}

	const float length = character->PlayAnimMontage(chosen, PlayRate, SlotName);
	if (!WaitForMontageToFinish)
	{
		return EBTNodeResult::Succeeded;
	}

	if (length <= 0.0f)
	{
		return EBTNodeResult::Succeeded;
	}

	// Bind to montage end and finish the task from the delegate
	UAnimInstance* animInstance = anim;
	if (IsValid(animInstance))
	{
		TWeakObjectPtr<UBehaviorTreeComponent> weakBT(&ownerComp);
		FOnMontageEnded endDelegate;
		endDelegate.BindLambda([this, weakBT, animInstance](UAnimMontage* montage, bool bInterrupted)
		{
			if (!weakBT.IsValid())
			{
				return;
			}
			UBehaviorTreeComponent* bt = weakBT.Get();
			OnMontageEnded(montage, bInterrupted, bt, animInstance);
		});
		MontageEndedDelegate = endDelegate;
		animInstance->Montage_SetEndDelegate(endDelegate, chosen);
	}

	return EBTNodeResult::InProgress;
}

void UJGTaskNode_PlayWeightedMontage::OnMontageEnded(UAnimMontage* montage, bool bInterrupted, UBehaviorTreeComponent* ownerComp, UAnimInstance* animInstance)
{
	if (!IsValid(ownerComp))
	{
		return;
	}

	if (IsValid(animInstance) && IsValid(montage))
	{
		animInstance->Montage_SetEndDelegate(MontageEndedDelegate, montage);
	}

	FinishLatentTask(*ownerComp, EBTNodeResult::Succeeded);
}

FString UJGTaskNode_PlayWeightedMontage::GetStaticDescription() const
{
	return FString::Printf(TEXT("Play Weighted Montage\nMontages: %d\nPlayRate: %.2f\nStop Existing: %s\nWait For Finish: %s (delegate)"),
		Montages.Num(),
		PlayRate,
		StopExistingMontages ? TEXT("true") : TEXT("false"),
		WaitForMontageToFinish ? TEXT("true") : TEXT("false"));
}


