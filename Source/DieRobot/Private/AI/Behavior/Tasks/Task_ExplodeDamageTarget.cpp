// Property of Paracosm.


#include "AI/Behavior/Tasks/Task_ExplodeDamageTarget.h"

#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BuildSystem/BuildingComponents/TimberBuildingComponentBase.h"
#include "Character/TimberSeeda.h"
#include "Character/Enemies/TimberEnemyCharacter.h"

UTask_ExplodeDamageTarget::UTask_ExplodeDamageTarget()
{
	bCreateNodeInstance = true;
	bNotifyTaskFinished = true;
	NodeName = "Explode Damage Target";
	
}

EBTNodeResult::Type UTask_ExplodeDamageTarget::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{

	BehaviorTreeComponent = &OwnerComp;
	if (!BehaviorTreeComponent) return EBTNodeResult::Failed;
	//UE_LOG(LogTemp, Warning, TEXT("Behavior Tree Component is Valid."));
	
	BlackboardComponent = BehaviorTreeComponent->GetBlackboardComponent();
	if (!BlackboardComponent) return EBTNodeResult::Failed;
	//UE_LOG(LogTemp, Warning, TEXT("Blackboard Component is Valid."));

	//Can be either a Buildable or Data Seed
	TargetActor = Cast<AActor>(BlackboardComponent->GetValueAsObject(TargetKey.SelectedKeyName));
	if (!TargetActor) return EBTNodeResult::Failed;
	//UE_LOG(LogTemp, Warning, TEXT("Target Actor is Valid."));

	//Referring to the Controlled Carbonite Character
	SelfEnemyCharacter = Cast<ATimberEnemyCharacter>(BlackboardComponent->GetValueAsObject(SelfActorKey.SelectedKeyName));
	if (!SelfEnemyCharacter) return EBTNodeResult::Failed;
	//UE_LOG(LogTemp, Warning, TEXT("Self Actor is Valid."));

	//UE_LOG(LogTemp, Warning, TEXT("1.UTask_ExplodeDamageTarget- Execute() - Executing Damage on Actor: %s"), *TargetActor->GetName());
	HandleDamage(TargetActor, SelfEnemyCharacter);
	
	return EBTNodeResult::Succeeded;
}

void UTask_ExplodeDamageTarget::OnTaskFinished(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory,
	EBTNodeResult::Type TaskResult)
{
	//Destroy Actor

	HandleCharacterDestruction(SelfEnemyCharacter);
	
	Super::OnTaskFinished(OwnerComp, NodeMemory, TaskResult);
}

void UTask_ExplodeDamageTarget::HandleDamage(AActor* ActorToDamage, ATimberEnemyCharacter* SelfActorEnemy)
{
	//Damage here can potentially be set on the Owning Actor. Maybe each type of Enemy applies different damage and the value is stored on the Enemy Character?
	//UE_LOG(LogTemp, Warning, TEXT("2.UTask_ExplodeDamageTarget - HandleDamage() - Executing Damage on Actor: %s"), *ActorToDamage->GetName());
	ATimberSeeda* DataSeed = Cast<ATimberSeeda>(ActorToDamage);
	if (DataSeed)
	{
		DataSeed->TakeDamage_Seeda(DataSeedDamageAmount);
		return;
	}
	
	//UE_LOG(LogTemp, Warning, TEXT("Could not Cast to Data Seed in HandleDamage()."));
	

	ATimberBuildingComponentBase* BuildableStructure = Cast<ATimberBuildingComponentBase>(ActorToDamage);
	if (BuildableStructure)
	{
		BuildableStructure->BuildingComponentTakeDamage(BuildableDamageAmount, SelfActorEnemy);
	}

	//UE_LOG(LogTemp, Warning, TEXT("Could not Cast to BuildingComponentBase in HandleDamage()."));
	
}

void UTask_ExplodeDamageTarget::HandleCharacterDestruction(ATimberEnemyCharacter* ActorForDestruction)
{
	//Calls the destruction function on the Actor.
	if (ActorForDestruction)
	{
		UE_LOG(LogTemp, Warning, TEXT("UTask_ExplodeDamageTarget - Executing Destruction on Actor: %s"), *ActorForDestruction->GetName());
		ActorForDestruction->SelfDestruct();
	}
}
