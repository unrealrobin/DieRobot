// Property of Paracosm.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "Task_PlayStandardAttackMontage.generated.h"

/**
 * 
 */
UCLASS()
class DIEROBOT_API UTask_PlayStandardAttackMontage : public UBTTaskNode
{
	GENERATED_BODY()

public:

	UTask_PlayStandardAttackMontage();

	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual EBTNodeResult::Type AbortTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual void OnTaskFinished(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, EBTNodeResult::Type TaskResult) override;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "TaskData")
	bool bPlayRandomSection = false;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "TaskData")
	FBlackboardKeySelector SelfActorKey;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "TaskData")
	FName SectionName = NAME_None;

	UFUNCTION()
	void OnMontageBlendOut(UAnimMontage* Montage, bool bInterrupted);

protected:
	void CleanUpDelegateBindings();

	UPROPERTY()
	UBehaviorTreeComponent* OwnerBehaviorTreeComp;

	UPROPERTY()
	UBlackboardComponent* BlackboardComp = nullptr;

	
	
};
