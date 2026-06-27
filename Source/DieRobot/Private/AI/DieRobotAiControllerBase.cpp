// Property of Paracosm Industries.


#include "AI/DieRobotAiControllerBase.h"

#include "NavigationSystem.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Character/Enemies/DieRobotEnemyCharacter.h"
#include "Components/CapsuleComponent.h"
#include "Components/Navigation/NavigationHelperComponent.h"
#include "GameFramework/CharacterMovementComponent.h"


ADieRobotAiControllerBase::ADieRobotAiControllerBase()
{
	BehaviorTreeComponent = CreateDefaultSubobject<UBehaviorTreeComponent>("BehaviorTree Component");
	BlackboardComponent = CreateDefaultSubobject<UBlackboardComponent>("Blackboard Component");

}

void ADieRobotAiControllerBase::BeginPlay()
{
	Super::BeginPlay();
	
	
}


void ADieRobotAiControllerBase::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);
	
	OwningCharacter = Cast<ADieRobotEnemyCharacter>(InPawn);
	
}
