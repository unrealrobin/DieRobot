// Property of Paracosm Industries. Dont use my shit.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "Task_Drones_FireProjectiles.generated.h"

/**
 * 
 */
UCLASS()
class DIEROBOT_API UTask_Drones_FireProjectiles : public UBTTaskNode
{
	GENERATED_BODY()

public:
	UTask_Drones_FireProjectiles();

	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

};
