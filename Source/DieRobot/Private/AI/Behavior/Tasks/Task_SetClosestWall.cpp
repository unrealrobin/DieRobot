// Property of Paracosm Industries.


#include "AI/Behavior/Tasks/Task_SetClosestWall.h"

#include "AI/DieRobotAiControllerBase.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BuildSystem/BuildingComponents/DieRobotHorizontalBuildingComponent.h"
#include "BuildSystem/buildingComponents/DieRobotVerticalBuildingComponent.h"
#include "BuildSystem/Ramps/RampBase.h"
#include "Character/Enemies/DieRobotEnemyCharacter.h"
#include "Kismet/KismetSystemLibrary.h"

UTask_SetClosestWall::UTask_SetClosestWall()
{
	NodeName = "AiSetClosestWall";
}

EBTNodeResult::Type UTask_SetClosestWall::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	Super::ExecuteTask(OwnerComp, NodeMemory);

	ADieRobotAiControllerBase* AIControllerBase = Cast<ADieRobotAiControllerBase>(OwnerComp.GetAIOwner());
	if (!AIControllerBase) return EBTNodeResult::Failed;

	ADieRobotEnemyCharacter* AiEnemyCharacter = Cast<ADieRobotEnemyCharacter>(AIControllerBase->GetPawn());
	if (!AiEnemyCharacter) return EBTNodeResult::Failed;

	UBlackboardComponent* BlackboardComponent = OwnerComp.GetBlackboardComponent();
	if (!BlackboardComponent) return EBTNodeResult::Failed;

	TArray<AActor*> WallsOrRampsArray;

	//Gets all vertical building components within RadiusToCheck (Set on the Task in the Editor)
	bool bHits = UKismetSystemLibrary::SphereOverlapActors(GetWorld(),
		AiEnemyCharacter->GetActorLocation(),
		RadiusToCheck,
		TArray<TEnumAsByte<EObjectTypeQuery>>(),
		ABuildableBase::StaticClass(),
		TArray<AActor*>(),
		WallsOrRampsArray);
	/*for (AActor* HitActors : WallsOrRampsArray)
	{
		UE_LOG(LogTemp, Warning, TEXT("Hit Actor: %s"), *HitActors->GetName());
	}*/
	//DrawDebugSphere(GetWorld(), AiEnemyCharacter->GetActorLocation(), 500,12, FColor::Green, false, 5.0f);

	/*if (bHits)
	{
		UE_LOG(LogTemp, Warning, TEXT("Found Vertical Building Components."))
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("No Matching Building Components Found."));
	}*/
	
	//Finding the Closest Wall
	if (bHits)
	{
		for (AActor* Hit : WallsOrRampsArray)
		{
			if (ABuildableBase* HitBuildable = Cast<ABuildableBase>(Hit))
			{
				if (HitBuildable->BuildableType != EBuildableType::Environment)
				{
					//If this hit Returns A Wall Base Class or a Ramp Base Class
					if (Cast<ADieRobotVerticalBuildingComponent>(HitBuildable) || Cast<ARampBase>(HitBuildable))
					{
						float Distance = FVector::Dist(AiEnemyCharacter->GetActorLocation(), HitBuildable->GetActorLocation());
						if (Distance < MatchStruct.Distance)
						{
							MatchStruct.WallorRamp = HitBuildable;
							MatchStruct.Distance = Distance;
						}
					}
				}
			}
		}
	}
	else
	{
		return EBTNodeResult::Failed;
	}

	//Setting the Closest WallorRamp on the Blackboard
	BlackboardComponent->SetValueAsObject(FName("ClosestBuildingComponentActor"), Cast<ABuildableBase>(MatchStruct.WallorRamp));
	
	if (bShowDebugSphere)
	{
		if (MatchStruct.WallorRamp)
		{
			DrawDebugSphere(GetWorld(), MatchStruct.WallorRamp->GetActorLocation(), 100.0f, 36, FColor::Red, false, 5.0f);
		}
	}
	

	//resetting
	MatchStruct.WallorRamp = nullptr;
	MatchStruct.Distance = UE_MAX_FLT;
	
	return EBTNodeResult::Succeeded;
}
