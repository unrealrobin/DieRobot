// Property of Paracosm Industries.


#include "AI/TimberAiControllerBase.h"

#include "NavigationSystem.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Character/Enemies/TimberEnemyCharacter.h"
#include "Components/CapsuleComponent.h"
#include "Components/Navigation/NavigationHelperComponent.h"
#include "GameFramework/CharacterMovementComponent.h"


ATimberAiControllerBase::ATimberAiControllerBase()
{
	BehaviorTreeComponent = CreateDefaultSubobject<UBehaviorTreeComponent>("BehaviorTree Component");
	BlackboardComponent = CreateDefaultSubobject<UBlackboardComponent>("Blackboard Component");

}

void ATimberAiControllerBase::BeginPlay()
{
	Super::BeginPlay();
	
	
}


void ATimberAiControllerBase::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);
	
	OwningCharacter = Cast<ATimberEnemyCharacter>(InPawn);
	
}
