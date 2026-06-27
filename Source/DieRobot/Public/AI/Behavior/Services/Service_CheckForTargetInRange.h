// Property of Paracosm.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTService.h"
#include "Service_CheckForTargetInRange.generated.h"

/**
 * 
 */
UCLASS()
class DIEROBOT_API UService_CheckForTargetInRange : public UBTService
{
	GENERATED_BODY()

public:
	
	UService_CheckForTargetInRange();

	//This is the range we want to check if the target is in.
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Service")
	float RangeToCheck = 100.0f;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Service")
	FBlackboardKeySelector SelfActorKey;

	//THis is the Target we Check the range for. Must be set before this service is called.
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Service")
	FBlackboardKeySelector TargetActorKey;

	//The Key we want to store the result of the check in.
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Service")
	FBlackboardKeySelector bIsTargetInRangeKey;

protected:
	
	virtual void TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;
	virtual void OnBecomeRelevant(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override; 
	virtual void OnCeaseRelevant(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override; 

private:
	
	void CheckRangeToTarget(UBehaviorTreeComponent& OwnerComp, AActor* TargetActorInstance);

	UPROPERTY()
	UBlackboardComponent* BlackboardComp = nullptr;

	UPROPERTY()
	AActor* TargetActor = nullptr;
	
	UPROPERTY()
	AActor* SelfActor = nullptr;
	
	bool bTargetIsInRange = false;

	

};
