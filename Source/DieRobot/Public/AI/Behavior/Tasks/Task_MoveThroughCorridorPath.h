// Property of Paracosm.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "Task_MoveThroughCorridorPath.generated.h"

class UPathFollowingComponent;
class ATimberAiControllerBase;
struct FPathFollowingResult;
class ATimberEnemyCharacter;
/**
 * 
 */
UCLASS()
class DIEROBOT_API UTask_MoveThroughCorridorPath : public UBTTaskNode
{
	GENERATED_BODY()

	UTask_MoveThroughCorridorPath();

public:
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual EBTNodeResult::Type AbortTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual void OnTaskFinished(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, EBTNodeResult::Type TaskResult) override;

	UPROPERTY(EditAnywhere, Category = "Blackboard")
	FBlackboardKeySelector TargetActorKey;

	UPROPERTY(EditAnywhere, Category = "Blackboard")
	FBlackboardKeySelector SelfActorKey;

	UPROPERTY(EditAnywhere, Category = "Blackboard")
	FBlackboardKeySelector bIsPartialPathKey;

	UPROPERTY(EditAnywhere, Category = "Blackboard")
	FBlackboardKeySelector LastPointInPathKey;
	

	void OnMoveFinished(FAIRequestID RequestID, const FPathFollowingResult& Result);

	int TotalCorridorPoints = 0;
	int NextCorridorPoint = 0;
	TArray<FVector> CorridorPathPoints;

	UPROPERTY(EditAnywhere, Category = "Blackboard")
	float AcceptanceRadius = 20.0f;
	
	bool bIsPathPartial = false;

private:
	UPROPERTY()
	UBehaviorTreeComponent* BehaviorTreeComponent = nullptr;

	UPROPERTY()
	ATimberAiControllerBase* AIControllerBase = nullptr;

	UPROPERTY()
	UPathFollowingComponent* PathFollowingComponent = nullptr;
};

/*
 * PathFollowingResultFlags
 *inline const Type Success = (1 << 0);
 *inline const Type Blocked = (1 << 1);
 *inline const Type OffPath = (1 << 2);
 *inline const Type UserAbort = (1 << 3);
 *inline const Type None = 0;
 *inline const Type OwnerFinished = (1 << 4);
 *inline const Type InvalidPath = (1 << 5);
 *inline const Type MovementStop = (1 << 6);
 *inline const Type NewRequest = (1 << 7);
 *inline const Type ForcedScript = (1 << 8);
 *inline const Type AlreadyAtGoal = (1 << 9);
*/