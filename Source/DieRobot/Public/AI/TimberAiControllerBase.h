// Property of Paracosm Industries.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "TimberAiControllerBase.generated.h"

class ATimberEnemyCharacter;
/**
 * 
 */
UCLASS()
class DIEROBOT_API ATimberAiControllerBase : public AAIController
{
	GENERATED_BODY()

public:
	ATimberAiControllerBase();

protected:
	virtual void BeginPlay() override;

	FTimerHandle CheckOnNavMeshTimerHandle;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Character Data")
	ATimberEnemyCharacter* OwningCharacter;

public:

	/* AI Setup */

	bool bIsOwnerOnNavMesh = true;

	virtual void OnPossess(APawn* InPawn) override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Ai Behavior")
	UBehaviorTreeComponent* BehaviorTreeComponent;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
	UBlackboardComponent* BlackboardComponent;
	
};
