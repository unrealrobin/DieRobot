// Property of Paracosm.


#include "AI/Behavior/Tasks/Task_MoveThroughCorridorPath.h"

#include "AI/TimberAiControllerBase.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Character/Enemies/TimberEnemyCharacter.h"
#include "Components/Navigation/NavigationHelperComponent.h"
#include "Navigation/PathFollowingComponent.h"

UTask_MoveThroughCorridorPath::UTask_MoveThroughCorridorPath()
{
	bCreateNodeInstance = true;
	bNotifyTaskFinished = true;
}

EBTNodeResult::Type UTask_MoveThroughCorridorPath::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	Super::ExecuteTask(OwnerComp, NodeMemory);

	BehaviorTreeComponent = &OwnerComp;
	
	AIControllerBase = Cast<ATimberAiControllerBase>(OwnerComp.GetAIOwner());
	if (!AIControllerBase) return EBTNodeResult::Failed;

	ATimberEnemyCharacter* AiEnemyCharacter = Cast<ATimberEnemyCharacter>(AIControllerBase->GetPawn());
	if (!AiEnemyCharacter) return EBTNodeResult::Failed;

	UBlackboardComponent* BlackboardComponent = OwnerComp.GetBlackboardComponent();
	if (!BlackboardComponent) return EBTNodeResult::Failed;

	ATimberEnemyCharacter* EnemyCharacter = Cast<ATimberEnemyCharacter>(BlackboardComponent->GetValueAsObject(SelfActorKey.SelectedKeyName));
	AActor* TargetActor = Cast<AActor>(BlackboardComponent->GetValueAsObject(TargetActorKey.SelectedKeyName));

	if (EnemyCharacter && EnemyCharacter->NavHelperComponent && TargetActor)
	{
		PathFollowingComponent = AIControllerBase->GetPathFollowingComponent();
		UE_LOG(LogTemp, Warning, TEXT("Task_MoveThroughCorridorPath: Retrieved Path to Target."));
		if (AIControllerBase && PathFollowingComponent)
		{
			CorridorPathPoints = EnemyCharacter->NavHelperComponent->GetCorridorPathPoints(
				EnemyCharacter->GetActorLocation(),
				TargetActor->GetActorLocation()
			);

			//Used to Set the value on the Blackboard. Value is used as a condition for the next Task in the Behavior Tree.
			bIsPathPartial = EnemyCharacter->NavHelperComponent->bIsLastPathPartial;
			BlackboardComponent->SetValueAsBool(bIsPartialPathKey.SelectedKeyName, bIsPathPartial);
			
			if (CorridorPathPoints.Num() > 0)
			{
				BlackboardComponent->SetValueAsVector(LastPointInPathKey.SelectedKeyName, CorridorPathPoints.Last());
				//Total Points in Corridor Path.
				TotalCorridorPoints = CorridorPathPoints.Num();
				//Callback for when the Move to the next point is finished
				PathFollowingComponent->OnRequestFinished.AddUObject(this, &UTask_MoveThroughCorridorPath::OnMoveFinished);
				//Calling Initial Move to the first point in the path.
				AIControllerBase->MoveToLocation(CorridorPathPoints[0], AcceptanceRadius);

				//Latent Task
				//UE_LOG(LogTemp, Warning, TEXT("Moving through Corridor in Progress."));
				return EBTNodeResult::InProgress;
			}
		}
	}
	//UE_LOG(LogTemp, Warning, TEXT("Initial Fail"));
	return EBTNodeResult::Failed;
}

EBTNodeResult::Type UTask_MoveThroughCorridorPath::AbortTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	if (AIControllerBase && PathFollowingComponent)
	{
		AIControllerBase->StopMovement();

		//Unbinding on Abort.
		PathFollowingComponent->OnRequestFinished.RemoveAll(this);
	}

	FinishLatentTask(*BehaviorTreeComponent, EBTNodeResult::Aborted);
	UE_LOG(LogTemp, Warning, TEXT("Move through Corridor Aborted."));
	return EBTNodeResult::Aborted;
	//return Super::AbortTask(OwnerComp, NodeMemory);
}

void UTask_MoveThroughCorridorPath::OnTaskFinished(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory,
	EBTNodeResult::Type TaskResult)
{
	Super::OnTaskFinished(OwnerComp, NodeMemory, TaskResult);

	CorridorPathPoints.Empty();
	TotalCorridorPoints = 0;
	NextCorridorPoint = 0;

	UE_LOG(LogTemp, Warning, TEXT("Task Finished, Reseting Corridor Waypoint Data. Result: %s"), *UEnum::GetValueAsString(TaskResult));
}


void UTask_MoveThroughCorridorPath::OnMoveFinished(FAIRequestID RequestID, const FPathFollowingResult& Result)
{
	if (Result.Code == EPathFollowingResult::Success)
	{
		NextCorridorPoint++;
		if (NextCorridorPoint < TotalCorridorPoints)
		{
			if (AIControllerBase)
			{
				AIControllerBase->MoveToLocation(CorridorPathPoints[NextCorridorPoint], AcceptanceRadius);
				return;
			}
		}
		//UE_LOG(LogTemp, Warning, TEXT("Finished Moving through Path."));
		FinishLatentTask(*BehaviorTreeComponent, EBTNodeResult::Succeeded );
	}
	
	if (Result.Code != EPathFollowingResult::Success)
	{
		switch (Result.Code)
		{
		case EPathFollowingResult::Aborted:
			UE_LOG(LogTemp, Warning, TEXT("Path Following Aborted."));
			break;
		case EPathFollowingResult::Blocked:
			UE_LOG(LogTemp, Warning, TEXT("Path Following Blocked."));
			break;
		case EPathFollowingResult::OffPath:
			UE_LOG(LogTemp, Warning, TEXT("Path Following Off Path."));
			break;
		case EPathFollowingResult::Invalid:
			UE_LOG(LogTemp, Warning, TEXT("Path Following Invalid."));
			break;
		default:
			UE_LOG(LogTemp, Warning, TEXT("Path Following failure,  default case."));
			break;
		}
		//UE_LOG(LogTemp, Warning, TEXT("Failed to finish Moving through corridor."));
		FinishLatentTask(*BehaviorTreeComponent, EBTNodeResult::Failed );
	}
}
