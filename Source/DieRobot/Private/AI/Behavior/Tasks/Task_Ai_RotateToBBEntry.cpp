// Property of Paracosm Industries. Dont use my shit.


#include "AI/Behavior/Tasks/Task_Ai_RotateToBBEntry.h"

#include "AI/TimberAiControllerBase.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Character/Enemies/TimberEnemyCharacter.h"

UTask_Ai_RotateToBBEntry::UTask_Ai_RotateToBBEntry()
{
	NodeName = "Task AI Rotate To BB Entry With Pitch";
}

EBTNodeResult::Type UTask_Ai_RotateToBBEntry::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{

	ATimberAiControllerBase* AIControllerBase = Cast<ATimberAiControllerBase>(OwnerComp.GetAIOwner());
	if (!AIControllerBase) return EBTNodeResult::Failed;

	ATimberEnemyCharacter* AiEnemyCharacter = Cast<ATimberEnemyCharacter>(AIControllerBase->GetPawn());
	if (!AiEnemyCharacter) return EBTNodeResult::Failed;

	UBlackboardComponent* BlackboardComponent = OwnerComp.GetBlackboardComponent();
	if (!BlackboardComponent) return EBTNodeResult::Failed;

	// Gets the Stored Vector Value of the passed in BB key (In Editor)
	FVector TargetLocation = BlackboardComponent->GetValueAsVector(TargetKey.SelectedKeyName);
	
	FVector AiLocation = AiEnemyCharacter->GetActorLocation();

	//Getting direction from one Location to Another Location and Normalizing it. (Length of 1)
	FVector Direction = (TargetLocation - AiLocation).GetSafeNormal();

	//If the Direction is nearly zero, then the AI is already facing the target.
	if (Direction.IsNearlyZero()) return EBTNodeResult::Failed;

	//Yaw and Pitch CAN be calculated from a Vector. Not Roll though.
	FRotator TargetRotation = Direction.Rotation();

	AiEnemyCharacter->SetActorRotation(TargetRotation);

	//For Smoother Rotation
	/*AiEnemyCharacter->SetActorRotation(FMath::RInterpTo(
	AiEnemyCharacter->GetActorRotation(),
	TargetRotation,
	GetWorld()->GetDeltaSeconds(),
	5.0f // Rotation speed multiplier
	));*/

	return EBTNodeResult::Succeeded;
}
