// Property of Paracosm Industries.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "Task_SetClosestWall.generated.h"

USTRUCT()
struct FMatchStruct
{
	GENERATED_BODY()

public:
	UPROPERTY()
	AActor* WallorRamp = nullptr;
	
	UPROPERTY()
	float Distance = UE_MAX_FLT;
	
};


/**
 * 
 */
UCLASS()
class DIEROBOT_API UTask_SetClosestWall : public UBTTaskNode
{
	GENERATED_BODY()

	UTask_SetClosestWall();

public:
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

	UPROPERTY()
	FMatchStruct MatchStruct;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TaskData")
	float RadiusToCheck = 300.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TaskData")
	bool bShowDebugSphere = false;
};
