// Property of Paracosm.


#include "AI/Behavior/Services/Service_CheckForTargetInRange.h"

#include "BehaviorTree/BlackboardComponent.h"
#include "EnvironmentQuery/EnvQueryTypes.h"

UService_CheckForTargetInRange::UService_CheckForTargetInRange()
{
	bCreateNodeInstance = true;
	bNotifyBecomeRelevant = true;
	bNotifyCeaseRelevant = true;
	bNotifyTick = true;
	Interval = 0.3f;
	RandomDeviation = 0.0f;
}

void UService_CheckForTargetInRange::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	Super::TickNode(OwnerComp, NodeMemory, DeltaSeconds);

	if (BlackboardComp && TargetActor && SelfActor && RangeToCheck > 0.0f)
	{
		CheckRangeToTarget(OwnerComp, TargetActor);
	}
	else
	{
		BlackboardComp = OwnerComp.GetBlackboardComponent();
		TargetActor = Cast<AActor>(BlackboardComp->GetValueAsObject(TargetActorKey.SelectedKeyName));
		SelfActor = Cast<AActor>(BlackboardComp->GetValueAsObject(SelfActorKey.SelectedKeyName));
	}
	
}

void UService_CheckForTargetInRange::OnBecomeRelevant(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	//Called once at the beginning of the Service Becoming Active
	Super::OnBecomeRelevant(OwnerComp, NodeMemory);

	BlackboardComp = OwnerComp.GetBlackboardComponent();
	if (!BlackboardComp) return;

	TargetActor = Cast<AActor>(BlackboardComp->GetValueAsObject(TargetActorKey.SelectedKeyName));
	if (!TargetActor) return;

	SelfActor = Cast<AActor>(BlackboardComp->GetValueAsObject(SelfActorKey.SelectedKeyName));
	if (!SelfActor) return;
	
}

void UService_CheckForTargetInRange::OnCeaseRelevant(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	Super::OnCeaseRelevant(OwnerComp, NodeMemory);
}

void UService_CheckForTargetInRange::CheckRangeToTarget(UBehaviorTreeComponent& OwnerComp, AActor* TargetActorInstance)
{

	FVector TargetLocation = TargetActorInstance->GetActorLocation();
	FVector SelfLocation = SelfActor->GetActorLocation();
	
	float Distance = FVector::Dist(TargetLocation, SelfLocation);
	//UE_LOG(LogTemp, Warning, TEXT("Distance between Self and Target: %f"), Distance);
	
	if (Distance < RangeToCheck && !bTargetIsInRange)
	{
		//UE_LOG(LogTemp, Warning, TEXT("Target is in range!"));
		BlackboardComp->SetValueAsBool(bIsTargetInRangeKey.SelectedKeyName, true);
		bTargetIsInRange = true;
	}
	else if (Distance > RangeToCheck && bTargetIsInRange)
	{
		//UE_LOG(LogTemp, Warning, TEXT("Target is not in range!"));
		BlackboardComp->SetValueAsBool(bIsTargetInRangeKey.SelectedKeyName, false);
		bTargetIsInRange = false;
	}
}
