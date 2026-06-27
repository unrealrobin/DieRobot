// Property of Paracosm.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "Task_ExplodeDamageTarget.generated.h"

class ATimberEnemyCharacter;
/**
 * 
 */
UCLASS()
class DIEROBOT_API UTask_ExplodeDamageTarget : public UBTTaskNode
{
	GENERATED_BODY()

public:

	UTask_ExplodeDamageTarget();

	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

	virtual void OnTaskFinished(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, EBTNodeResult::Type TaskResult) override;

	UPROPERTY()
	UBehaviorTreeComponent* BehaviorTreeComponent = nullptr;

	UPROPERTY()
	UBlackboardComponent* BlackboardComponent = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Blackboard")
	FBlackboardKeySelector SelfActorKey;

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "Blackboard")
	ATimberEnemyCharacter* SelfEnemyCharacter = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Blackboard")
	FBlackboardKeySelector TargetKey;

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "Blackboard")
	AActor* TargetActor = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Blackboard")
	float DataSeedDamageAmount = 20.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Blackboard")
	float BuildableDamageAmount = 50.0f;
	
private:

	UFUNCTION()
	void HandleDamage(AActor* ActorToDamage, ATimberEnemyCharacter* SelfActorEnemy);

	UFUNCTION()
	void HandleCharacterDestruction(ATimberEnemyCharacter* ActorForDestruction);
};
