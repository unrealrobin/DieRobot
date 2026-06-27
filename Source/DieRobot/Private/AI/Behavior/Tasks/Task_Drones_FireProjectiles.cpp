// Property of Paracosm Industries. Dont use my shit.


#include "AI/Behavior/Tasks/Task_Drones_FireProjectiles.h"

#include "AI/TimberAiControllerBase.h"
#include "Character/Enemies/FloaterDrones.h"

UTask_Drones_FireProjectiles::UTask_Drones_FireProjectiles()
{
	NodeName = "Drones Fire Projectiles";
}

EBTNodeResult::Type UTask_Drones_FireProjectiles::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	ATimberAiControllerBase* DroneController = Cast<ATimberAiControllerBase>(OwnerComp.GetAIOwner());
	if (!DroneController)
	{
		UE_LOG(LogTemp, Warning, TEXT("Task - Drones Fire Projectiles - Could not Cast to Controller."));
		return EBTNodeResult::Failed;
	}

	AFloaterDrones* DroneCharacter = Cast<AFloaterDrones>(DroneController->GetPawn());
	if (!DroneCharacter)
	{
		UE_LOG(LogTemp, Warning, TEXT("Task - Drones Fire Projectiles - Could not Cast to Character."));
		return EBTNodeResult::Failed;
	}

	//Potentially move this to Editor through BlackBoard for Readability.
	if (DroneCharacter->bIsLolaStunnedLocal)
	{
		UE_LOG(LogTemp, Warning, TEXT("Task - Drones Fire Projectiles - Lola is Stunned. Drones will not fire."));
		return EBTNodeResult::Failed;
	}
	
	//Time Between Shots must be handled with the Wait Node in the Behavior tree.
	DroneCharacter->FireProjectile();

	return EBTNodeResult::Succeeded;
}
