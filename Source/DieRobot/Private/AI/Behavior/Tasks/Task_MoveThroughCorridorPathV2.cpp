// Property of Paracosm.
// Property of Paracosm.

#include "AI/Behavior/Tasks/Task_MoveThroughCorridorPathV2.h"
#include "AI/TimberAiControllerBase.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Character/Enemies/TimberEnemyCharacter.h"
#include "Components/Navigation/NavigationHelperComponent.h"
#include "Navigation/PathFollowingComponent.h"
#include "BuildSystem/BuildableBase.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"
#include "Kismet/KismetSystemLibrary.h"

UTask_MoveThroughCorridorPathV2::UTask_MoveThroughCorridorPathV2()
{
    NodeName = "Move Through Corridor Path V2";
    bCreateNodeInstance = true;
    bNotifyTaskFinished = true;
}


EBTNodeResult::Type UTask_MoveThroughCorridorPathV2::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
    Super::ExecuteTask(OwnerComp, NodeMemory);

    // Cache core components
    BehaviorTreeComponent = &OwnerComp;
    AIControllerBase = Cast<ATimberAiControllerBase>(OwnerComp.GetAIOwner());
    
    if (!AIControllerBase)
    {
        //UE_LOG(LogTemp, Error, TEXT("Task_MoveThroughCorridorPathV2: Failed to get AI Controller"));
        return EBTNodeResult::Failed;
    }

    EnemyCharacter = Cast<ATimberEnemyCharacter>(AIControllerBase->GetPawn());
    if (!EnemyCharacter)
    {
        //UE_LOG(LogTemp, Error, TEXT("Task_MoveThroughCorridorPathV2: Failed to get Enemy Character"));
        return EBTNodeResult::Failed;
    }

    PathFollowingComponent = AIControllerBase->GetPathFollowingComponent();
    if (!PathFollowingComponent)
    {
        //UE_LOG(LogTemp, Error, TEXT("Task_MoveThroughCorridorPathV2: Failed to get PathFollowing Component"));
        return EBTNodeResult::Failed;
    }

    CurrentMoveRequestID = FAIRequestID::InvalidRequest;


    // Initialize path and start movement
    InitializePath();
    
    if (CorridorPathPoints.Num() == 0)
    {
        SetBlackboardValue(bHasValidPathKey.SelectedKeyName, false);
        //UE_LOG(LogTemp, Warning, TEXT("Task_MoveThroughCorridorPathV2: No valid path found"));
        return EBTNodeResult::Failed;
    }

    //Binding to function when reaching the next point.
    PathFollowingComponent->OnRequestFinished.AddUObject(this, &UTask_MoveThroughCorridorPathV2::OnMoveFinished);

    if (bEnableDebugDrawing)
    {
        DrawDebugPath();
    }
    
    MoveToNextWaypoint();

    return EBTNodeResult::InProgress;
}

EBTNodeResult::Type UTask_MoveThroughCorridorPathV2::AbortTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
    if (AIControllerBase)
    {
        AIControllerBase->StopMovement();
    }

    if (PathFollowingComponent)
    {
        PathFollowingComponent->OnRequestFinished.RemoveAll(this);
    }

    // Clear timer if active
    if (GetWorld() && GetWorld()->GetTimerManager().IsTimerActive(BlockedRetryTimer))
    {
        GetWorld()->GetTimerManager().ClearTimer(BlockedRetryTimer);
    }

    ClearPathData();
    //UE_LOG(LogTemp, Warning, TEXT("Task_MoveThroughCorridorPathV2: Task Aborted"));
    
    return EBTNodeResult::Aborted;
}

void UTask_MoveThroughCorridorPathV2::OnTaskFinished(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, EBTNodeResult::Type TaskResult)
{
    Super::OnTaskFinished(OwnerComp, NodeMemory, TaskResult);

    // Clean up callbacks
    if (PathFollowingComponent)
    {
        PathFollowingComponent->OnRequestFinished.RemoveAll(this);
    }

    // Clear timer
    if (GetWorld() && GetWorld()->GetTimerManager().IsTimerActive(BlockedRetryTimer))
    {
        GetWorld()->GetTimerManager().ClearTimer(BlockedRetryTimer);
    }

    ClearPathData();
    //LogPathState(FString::Printf(TEXT("Task Finished - Result: %s"), *UEnum::GetValueAsString(TaskResult)));
}

void UTask_MoveThroughCorridorPathV2::OnMoveFinished(FAIRequestID RequestID, const FPathFollowingResult& Result)
{
    bool bIsSuccess = (Result.Code == EPathFollowingResult::Success);
    bool bIsBlocked = (Result.Code == EPathFollowingResult::Blocked);
    bool bIsAborted = (Result.Code == EPathFollowingResult::Aborted);
    bool bIsInvalid = (Result.Code == EPathFollowingResult::Invalid);
    
    //UE_LOG(LogTemp, Warning, TEXT("Move Finished - Result: %s"), *UEnum::GetValueAsString(Result.Code));
    if (bIsSuccess)
    {
        //UE_LOG(LogTemp, Warning, TEXT("Move To Point Successfull."));
        HandleSuccessfulMove();
    }
    else if (bIsBlocked || bIsInvalid)
    {
        //UE_LOG(LogTemp, Warning, TEXT("Move To Point BLOCKED."));
        HandleBlockedPath();
    }
    else if (bIsAborted)
    {
        if (bRecalculateOnBlocked && BlockedAttempts < MaxBlockedAttempts)
        {
            BlockedAttempts++;
            RecalculatePathFromCurrentPosition();
        }
        else
        {
            FinishLatentTask(*BehaviorTreeComponent, EBTNodeResult::Failed);
        }
    }
}

void UTask_MoveThroughCorridorPathV2::InitializePath()
{
    if (!EnemyCharacter || !EnemyCharacter->NavHelperComponent)
    {
        //UE_LOG(LogTemp, Error, TEXT("Task_MoveThroughCorridorPathV2: Missing NavHelper Component"));
        return;
    }

    UBlackboardComponent* BlackboardComp = BehaviorTreeComponent->GetBlackboardComponent();
    if (!BlackboardComp)
    {
        //UE_LOG(LogTemp, Error, TEXT("Task_MoveThroughCorridorPathV2: No Blackboard Component"));
        return;
    }

    // Get target from blackboard
    AActor* TargetActor = Cast<AActor>(BlackboardComp->GetValueAsObject(TargetActorKey.SelectedKeyName));
    if (!TargetActor)
    {
        //UE_LOG(LogTemp, Error, TEXT("Task_MoveThroughCorridorPathV2: No Target Actor in Blackboard"));
        return;
    }

    // Calculate path using NavigationHelper
    FPathResult PathResult = EnemyCharacter->NavHelperComponent->CalculatePathToTarget(
        EnemyCharacter->GetActorLocation(),
        TargetActor->GetActorLocation()
    );

    //PathResult.bIsValid &&
    if ( PathResult.PathPoints.Num() > 0)
    {
        
        CorridorPathPoints = PathResult.PathPoints;
        TotalCorridorPoints = CorridorPathPoints.Num();
        CurrentWaypointIndex = 0;
        BlockedAttempts = 0;
        bIsPathPartial = PathResult.bIsPartial;

        // Update blackboard values
        SetBlackboardValue(bHasValidPathKey.SelectedKeyName, PathResult.bIsValid); // Always True in this Case
        SetBlackboardValue(bIsPartialPathKey.SelectedKeyName, bIsPathPartial);
        SetBlackboardValue(LastPointInPathKey.SelectedKeyName, CorridorPathPoints.Last());
        SetBlackboardValue(bMovementBlockedKey.SelectedKeyName, false); // Always False in this Case
        SetBlackboardValue(bShouldDestroyWallKey.SelectedKeyName, false); // Always False in this Case
    }
    else
    {
        //This Failure is handled in the outer Execute function
        //Should also never happen as the CalculatePathToTarget function returns partial paths.
        SetBlackboardValue(bHasValidPathKey.SelectedKeyName, false);
        UE_LOG(LogTemp, Error, TEXT("Task_MoveThroughCorridorPathV2: Failed to calculate valid path"));
    }
}

FAdjustedPathFollowingTarget UTask_MoveThroughCorridorPathV2::ApplyAdjustments(FVector NextWaypoint)
{
    FAdjustedPathFollowingTarget Temp;
    //Handle Adjustments and Offset Randmization for Path Point and Acceptance Radius
    
    if (CurrentWaypointIndex < CorridorPathPoints.Num() - 1)
    {
        Temp.AdjustedAcceptanceRadius = AcceptanceRadius * 3;
    }
    else
    {
        Temp.AdjustedAcceptanceRadius = AcceptanceRadius;
    }
    AdjustedPathFollowingTarget = Temp;

     if (NextWaypoint != FVector::ZeroVector)
     {
         float AdjustedX = NextWaypoint.X + (FMath::FRandRange(-0.5f, 0.5f) * 50.0f);
         float AdjustedY = NextWaypoint.Y + (FMath::FRandRange(-0.5f, 0.5f) * 50.0f);

         FVector AdjustedLocation = FVector(AdjustedX, AdjustedY, NextWaypoint.Z);
        AdjustedPathFollowingTarget.AdjustedVector = AdjustedLocation;
     }


    return AdjustedPathFollowingTarget;
}

void UTask_MoveThroughCorridorPathV2::MoveToNextWaypoint()
{
    if (!AIControllerBase || !IsPathValid() || CurrentWaypointIndex >= TotalCorridorPoints)
    {
        //UE_LOG(LogTemp, Error, TEXT("MoveToNextWaypoint: Invalid state - aborting"));
        return;
    }

    FVector NextWaypoint = CorridorPathPoints[CurrentWaypointIndex];

    //FAdjustedPathFollowingTarget AdjustedData = ApplyAdjustments(NextWaypoint);
    
    //TODO:: Plug-in the Correct Data.
    
    if (PathFollowingComponent)
    {
        //Base Values
        AIControllerBase->MoveToLocation(NextWaypoint, AcceptanceRadius);
        //DrawDebugSphere(GetWorld(), NextWaypoint, 20, 12, FColor::Red, false, 2.0f);
        //Uses Random Offsets Generated in Apply Adjustments()
        //AIControllerBase->MoveToLocation(AdjustedData.AdjustedVector, AdjustedData.AdjustedAcceptanceRadius);
        CurrentMoveRequestID = PathFollowingComponent->GetCurrentRequestId();
        //UE_LOG(LogTemp, Error, TEXT("MoveToNextWaypoint() - CurrentMoveRequest: %d"), CurrentMoveRequestID.GetID());
    }
}

void UTask_MoveThroughCorridorPathV2::HandleSuccessfulMove()
{
    CurrentWaypointIndex++;
    BlockedAttempts = 0; // Reset blocked attempts on successful move
    //UE_LOG(LogTemp, Warning, TEXT("Task_MoveThroughCorridorPathV2: Successfully moved to waypoint %d/%d"), CurrentWaypointIndex, TotalCorridorPoints );
    if (CurrentWaypointIndex >= TotalCorridorPoints)
    {
        if (bIsPathPartial)
        {
            //Should be at last point in the path of a partial path.
            //UE_LOG(LogTemp, Warning, TEXT("Task_MoveThroughCorridorPathV2: At end of Partial Path. Run Recalculation or Handle Blockage."));
            //HandleBlockedPath();
            FinishLatentTask(*BehaviorTreeComponent, EBTNodeResult::Succeeded);
        }
        else
        {
            // Reached final destination
            // UE_LOG(LogTemp, Log, TEXT("Task_MoveThroughCorridorPathV2: Successfully reached destination"));
            FinishLatentTask(*BehaviorTreeComponent, EBTNodeResult::Succeeded);
        }
    }
    else
    {
        // Move to next waypoint
        MoveToNextWaypoint();
    }
}

void UTask_MoveThroughCorridorPathV2::HandleBlockedPath()
{
    //HandleMultipleBlockages();
    //Attempting to find another open route to the target - 3 Trys
    RecalculatePathFromCurrentPosition();
}

void UTask_MoveThroughCorridorPathV2::HandleMultipleBlockages()
{
    if (bAllowWallDestruction)
    {
        ABuildableBase* BlockingWall = FindBlockingWall();
        if (BlockingWall)
        {
            // Signal wall destruction system
            SetBlackboardValue(BlockingWallKey.SelectedKeyName, BlockingWall);
            SetBlackboardValue(bShouldDestroyWallKey.SelectedKeyName, true);
            
            //UE_LOG(LogTemp, Warning, TEXT("Task_MoveThroughCorridorPathV2: Requesting wall destruction"));
            FinishLatentTask(*BehaviorTreeComponent, EBTNodeResult::Succeeded); // Let other systems handle wall destruction
            return;
        }
    }

    if (bUseFallbackPath)
    {
        RecalculatePathFromCurrentPosition();
        if (CorridorPathPoints.Num() > 0)
        {
            bIsExecutingFallbackPath = true;
            CurrentWaypointIndex = 0;
            BlockedAttempts = 0;
            MoveToNextWaypoint();
            return;
        }
    }

    // Complete failure
    //UE_LOG(LogTemp, Error, TEXT("Task_MoveThroughCorridorPathV2: All pathfinding options exhausted"));
    FinishLatentTask(*BehaviorTreeComponent, EBTNodeResult::Failed);
}

ABuildableBase* UTask_MoveThroughCorridorPathV2::FindBlockingWall() const
{
    if (!EnemyCharacter)
    {
        return nullptr;
    }

    FVector StartLocation = EnemyCharacter->GetActorLocation();
    FVector ForwardDirection = EnemyCharacter->GetActorForwardVector();
    FVector EndLocation = StartLocation + (ForwardDirection * WallDetectionRadius);

    // Sphere trace for blocking walls
    TArray<FHitResult> HitResults;
    bool bHit = UKismetSystemLibrary::SphereTraceMulti(
        GetWorld(),
        StartLocation,
        EndLocation,
        50.0f, // Sphere radius
        UEngineTypes::ConvertToTraceType(ECC_WorldStatic),
        false, // Complex collision
        TArray<AActor*>(), // Actors to ignore
        EDrawDebugTrace::None,
        HitResults,
        true
    );

    if (bHit)
    {
        for (const FHitResult& Hit : HitResults)
        {
            ABuildableBase* Buildable = Cast<ABuildableBase>(Hit.GetActor());
            if (Buildable)
            {
                return Buildable;
            }
        }
    }

    return nullptr;
}

void UTask_MoveThroughCorridorPathV2::RecalculatePathFromCurrentPosition()
{
    if (!EnemyCharacter || !EnemyCharacter->NavHelperComponent)
    {
        return;
    }

    UBlackboardComponent* BlackboardComp = BehaviorTreeComponent->GetBlackboardComponent();
    AActor* TargetActor = Cast<AActor>(BlackboardComp->GetValueAsObject(TargetActorKey.SelectedKeyName));
    
    if (!TargetActor)
    {
        return;
    }

    // Recalculate path from current position
    FPathResult NewPathResult = EnemyCharacter->NavHelperComponent->CalculatePathToTarget(
        EnemyCharacter->GetActorLocation(),
        TargetActor->GetActorLocation()
    );

    if (NewPathResult.bIsValid && NewPathResult.PathPoints.Num() > 0)
    {
        CorridorPathPoints.Empty();
        CorridorPathPoints = NewPathResult.PathPoints;
        TotalCorridorPoints = CorridorPathPoints.Num();
        CurrentWaypointIndex = 0;
        bIsPathPartial = NewPathResult.bIsPartial;
        
         if (bEnableDebugDrawing)
         {
             DrawDebugPath();
         }
        
        UE_LOG(LogTemp, Log, TEXT("Task_MoveThroughCorridorPathV2: Successfully recalculated path"));
        MoveToNextWaypoint();
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("Task_MoveThroughCorridorPathV2: Failed to recalculate path"));
        FinishLatentTask(*BehaviorTreeComponent, EBTNodeResult::Failed);
    }
}

void UTask_MoveThroughCorridorPathV2::RetryMovementAfterDelay()
{
    if (GetWorld())
    {
        GetWorld()->GetTimerManager().SetTimer(
            BlockedRetryTimer,
            this,
            &UTask_MoveThroughCorridorPathV2::MoveToNextWaypoint,
            1.0f, // 1 second delay
            false
        );
    }
}

void UTask_MoveThroughCorridorPathV2::ClearPathData()
{
    CorridorPathPoints.Empty();
    CurrentWaypointIndex = 0;
    TotalCorridorPoints = 0;
    BlockedAttempts = 0;
    bIsExecutingFallbackPath = false;
    bIsPathPartial = false;
    CurrentMoveRequestID = FAIRequestID::InvalidRequest;
}

bool UTask_MoveThroughCorridorPathV2::IsPathValid() const
{
    return CorridorPathPoints.Num() > 0 && CurrentWaypointIndex < TotalCorridorPoints;
}

bool UTask_MoveThroughCorridorPathV2::IsAtDestination() const
{
    if (!EnemyCharacter || CorridorPathPoints.Num() == 0)
    {
        return false;
    }

    float Distance = FVector::Dist(EnemyCharacter->GetActorLocation(), CorridorPathPoints.Last());
    return Distance <= AcceptanceRadius;
}

bool UTask_MoveThroughCorridorPathV2::IsAtCurrentWaypoint() const
{
    if (!EnemyCharacter || !IsPathValid())
    {
        return false;
    }

    return GetDistanceToCurrentWaypoint() <= AcceptanceRadius;
}

float UTask_MoveThroughCorridorPathV2::GetDistanceToCurrentWaypoint() const
{
    if (!EnemyCharacter || !IsPathValid())
    {
        return -1.0f;
    }

    return FVector::Dist(EnemyCharacter->GetActorLocation(), GetCurrentWaypoint());
}

float UTask_MoveThroughCorridorPathV2::GetDistanceToTarget() const
{
    if (!EnemyCharacter || CorridorPathPoints.Num() == 0)
    {
        return -1.0f;
    }

    return FVector::Dist(EnemyCharacter->GetActorLocation(), CorridorPathPoints.Last());
}

FVector UTask_MoveThroughCorridorPathV2::GetCurrentWaypoint() const
{
    if (IsPathValid())
    {
        return CorridorPathPoints[CurrentWaypointIndex];
    }
    return FVector::ZeroVector;
}

FVector UTask_MoveThroughCorridorPathV2::GetTargetLocation() const
{
    UBlackboardComponent* BlackboardComp = BehaviorTreeComponent ? BehaviorTreeComponent->GetBlackboardComponent() : nullptr;
    if (!BlackboardComp)
    {
        return FVector::ZeroVector;
    }

    AActor* TargetActor = Cast<AActor>(BlackboardComp->GetValueAsObject(TargetActorKey.SelectedKeyName));
    return TargetActor ? TargetActor->GetActorLocation() : FVector::ZeroVector;
}

// Blackboard Helper Functions
void UTask_MoveThroughCorridorPathV2::SetBlackboardValue(const FName& KeyName, bool Value)
{
    UBlackboardComponent* BlackboardComp = BehaviorTreeComponent ? BehaviorTreeComponent->GetBlackboardComponent() : nullptr;
    if (BlackboardComp)
    {
        BlackboardComp->SetValueAsBool(KeyName, Value);
    }
}

void UTask_MoveThroughCorridorPathV2::SetBlackboardValue(const FName& KeyName, UObject* Value)
{
    UBlackboardComponent* BlackboardComp = BehaviorTreeComponent ? BehaviorTreeComponent->GetBlackboardComponent() : nullptr;
    if (BlackboardComp)
    {
        BlackboardComp->SetValueAsObject(KeyName, Value);
    }
}

void UTask_MoveThroughCorridorPathV2::SetBlackboardValue(const FName& KeyName, FVector Value)
{
    UBlackboardComponent* BlackboardComp = BehaviorTreeComponent ? BehaviorTreeComponent->GetBlackboardComponent() : nullptr;
    if (BlackboardComp)
    {
        BlackboardComp->SetValueAsVector(KeyName, Value);
    }
}

bool UTask_MoveThroughCorridorPathV2::GetBlackboardValue(const FName& KeyName, bool& OutValue) const
{
    UBlackboardComponent* BlackboardComp = BehaviorTreeComponent ? BehaviorTreeComponent->GetBlackboardComponent() : nullptr;
    if (BlackboardComp)
    {
        OutValue = BlackboardComp->GetValueAsBool(KeyName);
        return true;
    }
    return false;
}

UObject* UTask_MoveThroughCorridorPathV2::GetBlackboardValue(const FName& KeyName) const
{
    UBlackboardComponent* BlackboardComp = BehaviorTreeComponent ? BehaviorTreeComponent->GetBlackboardComponent() : nullptr;
    return BlackboardComp ? BlackboardComp->GetValueAsObject(KeyName) : nullptr;
}

// Debug Functions
void UTask_MoveThroughCorridorPathV2::LogPathState(const FString& Context) const
{
    UE_LOG(LogTemp, Log, TEXT("Task_MoveThroughCorridorPathV2 [%s]: Waypoint %d/%d, Blocked Attempts: %d, Partial: %s, Fallback: %s"),
           *Context,
           CurrentWaypointIndex + 1,
           TotalCorridorPoints,
           BlockedAttempts,
           bIsPathPartial ? TEXT("Yes") : TEXT("No"),
           bIsExecutingFallbackPath ? TEXT("Yes") : TEXT("No"));
}

void UTask_MoveThroughCorridorPathV2::DrawDebugPath() const
{
    if (!bEnableDebugDrawing || !GetWorld())
    {
        return;
    }

    for (int32 i = 0; i < CorridorPathPoints.Num() - 1; i++)
    {
        DrawDebugLine(
            GetWorld(),
            CorridorPathPoints[i],
            CorridorPathPoints[i + 1],
            FColor::Green,
            false,
            5.0f,
            0,
            3.0f
        );

        DrawDebugSphere(
            GetWorld(),
            CorridorPathPoints[i],
            AcceptanceRadius,
            12,
            FColor::Blue,
            false,
            5.0f
        );
    }

    // Draw final destination
    if (CorridorPathPoints.Num() > 0)
    {
        DrawDebugSphere(
            GetWorld(),
            CorridorPathPoints.Last(),
            AcceptanceRadius,
            12,
            FColor::Red,
            false,
            5.0f
        );
    }
}

void UTask_MoveThroughCorridorPathV2::DrawDebugCurrentWaypoint() const
{
    if (!bEnableDebugDrawing || !GetWorld() || !IsPathValid())
    {
        return;
    }

    DrawDebugSphere(
        GetWorld(),
        GetCurrentWaypoint(),
        AcceptanceRadius * 1.5f,
        12,
        FColor::Yellow,
        false,
        1.0f
    );
}

// Blueprint Debug Functions
FString UTask_MoveThroughCorridorPathV2::GetTaskStatus() const
{
    return FString::Printf(TEXT("Waypoint: %d/%d | Blocked: %d | Partial: %s | Fallback: %s"),
                          CurrentWaypointIndex + 1,
                          TotalCorridorPoints,
                          BlockedAttempts,
                          bIsPathPartial ? TEXT("Yes") : TEXT("No"),
                          bIsExecutingFallbackPath ? TEXT("Yes") : TEXT("No"));
}

void UTask_MoveThroughCorridorPathV2::ForceRecalculatePath()
{
    if (BehaviorTreeComponent && BehaviorTreeComponent->IsRunning())
    {
        //UE_LOG(LogTemp, Log, TEXT("Task_MoveThroughCorridorPathV2: Force recalculating path"));
        RecalculatePathFromCurrentPosition();
    }
}
