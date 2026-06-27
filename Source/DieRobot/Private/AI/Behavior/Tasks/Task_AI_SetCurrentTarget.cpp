// Property of Paracosm Industries


#include "AI/Behavior/Tasks/Task_AI_SetCurrentTarget.h"

#include "AI/DieRobotAiControllerBase.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Character/Enemies/DieRobotEnemyCharacter.h"

UTask_AI_SetCurrentTarget::UTask_AI_SetCurrentTarget()
{
	NodeName = "SetActorAsCurrentTarget";
}

EBTNodeResult::Type UTask_AI_SetCurrentTarget::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	Super::ExecuteTask(OwnerComp, NodeMemory);

	ADieRobotAiControllerBase* AIControllerBase = Cast<ADieRobotAiControllerBase>(OwnerComp.GetAIOwner());
	if (!AIControllerBase) return EBTNodeResult::Failed;

	ADieRobotEnemyCharacter* AiEnemyCharacter = Cast<ADieRobotEnemyCharacter>(AIControllerBase->GetPawn());
	if (!AiEnemyCharacter) return EBTNodeResult::Failed;

	UBlackboardComponent* BlackboardComponent = OwnerComp.GetBlackboardComponent();
	if (!BlackboardComponent) return EBTNodeResult::Failed;
	
	//Getting the Actor stored on the BB at the input Key. Key is set on the Task on the Behavior Tree
	AActor* CurrentTarget = Cast<AActor>(BlackboardComponent->GetValueAsObject(TargetKey.SelectedKeyName));
	if (CurrentTarget)
	{
		//Setting current target on Enemy AI Character
		Cast<ADieRobotEnemyCharacter>(AIControllerBase->GetPawn())->CurrentTarget = CurrentTarget;
		//UE_LOG(LogTemp, Warning, TEXT("Task_AI_SetCurrentTarget: Current Target is %s."), *CurrentTarget->GetName());
		return EBTNodeResult::Succeeded;
	}
	else
	{
		//UE_LOG(LogTemp, Warning, TEXT("Task_AI_SetCurrentTarget: Current Target is NULL."));
		return EBTNodeResult::Failed;
	}
}
