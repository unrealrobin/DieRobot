// Property of Paracosm Industries.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "Task_AI_SetCurrentTarget.generated.h"

/**
 * 
 */
UCLASS()
class DIEROBOT_API UTask_AI_SetCurrentTarget : public UBTTaskNode
{
	GENERATED_BODY()

	UTask_AI_SetCurrentTarget();

public:
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

	//Stores the BB Key of the Target Actor we want to Set
	UPROPERTY(EditAnywhere, Category = "Blackboard")
	FBlackboardKeySelector TargetKey;
};
