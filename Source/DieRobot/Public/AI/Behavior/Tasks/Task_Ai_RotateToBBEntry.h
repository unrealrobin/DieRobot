// Property of Paracosm Industries. Dont use my shit.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "Task_Ai_RotateToBBEntry.generated.h"

/**
 * 
 */
UCLASS()
class DIEROBOT_API UTask_Ai_RotateToBBEntry : public UBTTaskNode
{
	GENERATED_BODY()

public:

	UTask_Ai_RotateToBBEntry();

	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
protected:

	UPROPERTY(EditAnywhere, Category = "Blackboard")
	FBlackboardKeySelector TargetKey;
};
