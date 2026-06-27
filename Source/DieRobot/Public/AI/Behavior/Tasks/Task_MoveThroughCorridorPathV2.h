// Property of Paracosm.


// Property of Paracosm.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "AIController.h"
#include "Components/Navigation/NavigationHelperComponent.h" // Include to get FPathResult and EPathFailureReason
#include "Task_MoveThroughCorridorPathV2.generated.h"

class UPathFollowingComponent;
class ATimberAiControllerBase;
struct FPathFollowingResult;
class ATimberEnemyCharacter;
class ABuildableBase;

USTRUCT(BlueprintType)
struct FAdjustedPathFollowingTarget
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Path Following Target");
    FVector AdjustedVector = FVector::ZeroVector;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Path Following Target");
    float AdjustedAcceptanceRadius = 40.0f;
    
};

/**
 * Enhanced corridor path movement task with blocked handling and wall destruction support
 */
UCLASS()
class DIEROBOT_API UTask_MoveThroughCorridorPathV2 : public UBTTaskNode
{
    GENERATED_BODY()

public:
    UTask_MoveThroughCorridorPathV2();

    virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
    virtual EBTNodeResult::Type AbortTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
    virtual void OnTaskFinished(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, EBTNodeResult::Type TaskResult) override;

    // Blackboard key selectors
    UPROPERTY(EditAnywhere, Category = "Blackboard")
    FBlackboardKeySelector TargetActorKey;

    UPROPERTY(EditAnywhere, Category = "Blackboard")
    FBlackboardKeySelector SelfActorKey;

    //Path does not go all the way to Target
    UPROPERTY(EditAnywhere, Category = "Blackboard")
    FBlackboardKeySelector bIsPartialPathKey;

    //The last point in the path, stored as a Vector
    UPROPERTY(EditAnywhere, Category = "Blackboard")
    FBlackboardKeySelector LastPointInPathKey;

    //An Object to the wall that is blocking the path
    UPROPERTY(EditAnywhere, Category = "Blackboard")
    FBlackboardKeySelector BlockingWallKey;

    //If should destroy the Wall
    UPROPERTY(EditAnywhere, Category = "Blackboard")
    FBlackboardKeySelector bShouldDestroyWallKey;

    //If a path exisitings at all - Navigation Helper Component
    UPROPERTY(EditAnywhere, Category = "Blackboard")
    FBlackboardKeySelector bHasValidPathKey;

    //If Path has become Blocked during movement.
    UPROPERTY(EditAnywhere, Category = "Blackboard")
    FBlackboardKeySelector bMovementBlockedKey;

    // Path configuration
    UPROPERTY(EditAnywhere, Category = "Path Options")
    float AcceptanceRadius = 20.0f;

    UPROPERTY(EditAnywhere, Category = "Path Options")
    bool bAllowWallDestruction = true;

    UPROPERTY(EditAnywhere, Category = "Path Options")
    bool bRecalculateOnBlocked = false;

    UPROPERTY(EditAnywhere, Category = "Path Options")
    int32 MaxBlockedAttempts = 3;

    UPROPERTY(EditAnywhere, Category = "Path Options")
    float WallDetectionRadius = 150.0f;

    UPROPERTY(EditAnywhere, Category = "Path Options")
    bool bUseFallbackPath = true;

    UPROPERTY(EditAnywhere, Category = "Path Options")
    bool bEnableDebugDrawing = false;

    // Movement callback
    void OnMoveFinished(FAIRequestID RequestID, const FPathFollowingResult& Result);

private:
    // Core components
    UPROPERTY()
    UBehaviorTreeComponent* BehaviorTreeComponent = nullptr;

    UPROPERTY()
    ATimberAiControllerBase* AIControllerBase = nullptr;

    UPROPERTY()
    UPathFollowingComponent* PathFollowingComponent = nullptr;

    UPROPERTY()
    ATimberEnemyCharacter* EnemyCharacter = nullptr;

    // Path state
    TArray<FVector> CorridorPathPoints;
    int32 CurrentWaypointIndex = 0;
    int32 TotalCorridorPoints = 0;
    bool bIsExecutingFallbackPath = false;
    bool bIsPathPartial = false;

    /*Adjustments*/
    FAdjustedPathFollowingTarget AdjustedPathFollowingTarget;
    FAdjustedPathFollowingTarget ApplyAdjustments(FVector NextWaypoint);
    
    // Blocked handling state
    int32 BlockedAttempts = 0;
    FAIRequestID CurrentMoveRequestID;
    FTimerHandle BlockedRetryTimer;

    // Core movement functions
    void MoveToNextWaypoint();
    void HandleSuccessfulMove();
    FVector GetTargetLocation() const;
    
    // Path management
    void InitializePath();
    void RecalculatePathFromCurrentPosition();
    void ClearPathData();
    bool IsPathValid() const;
    
    // Blocked path handling
    void HandleBlockedPath();
    void HandleMultipleBlockages();
    ABuildableBase* FindBlockingWall() const;
    void RetryMovementAfterDelay();
    
    // Blackboard helpers
    void SetBlackboardValue(const FName& KeyName, bool Value);
    void SetBlackboardValue(const FName& KeyName, UObject* Value);
    void SetBlackboardValue(const FName& KeyName, FVector Value);
    bool GetBlackboardValue(const FName& KeyName, bool& OutValue) const;
    UObject* GetBlackboardValue(const FName& KeyName) const;
    
    // Utility functions
    bool IsAtDestination() const;
    bool IsAtCurrentWaypoint() const;
    float GetDistanceToCurrentWaypoint() const;
    float GetDistanceToTarget() const;
    FVector GetCurrentWaypoint() const;
    
    // Debug helpers
    void LogPathState(const FString& Context) const;
    void DrawDebugPath() const;
    void DrawDebugCurrentWaypoint() const;

public:
    // Public accessors for debugging/monitoring
    UFUNCTION(BlueprintCallable, Category = "Debug")
    int32 GetCurrentWaypointIndex() const { return CurrentWaypointIndex; }
    
    UFUNCTION(BlueprintCallable, Category = "Debug")
    int32 GetTotalWaypoints() const { return TotalCorridorPoints; }
    
    UFUNCTION(BlueprintCallable, Category = "Debug")
    bool IsExecutingFallbackPath() const { return bIsExecutingFallbackPath; }
    
    UFUNCTION(BlueprintCallable, Category = "Debug")
    TArray<FVector> GetCurrentPath() const { return CorridorPathPoints; }

    UFUNCTION(BlueprintCallable, Category = "Debug")
    FString GetTaskStatus() const;

    UFUNCTION(BlueprintCallable, Category = "Debug")
    void ForceRecalculatePath();
};

/*
 * PathFollowingResultFlags Reference:
 * inline const Type Success = (1 << 0);
 * inline const Type Blocked = (1 << 1);
 * inline const Type OffPath = (1 << 2);
 * inline const Type UserAbort = (1 << 3);
 * inline const Type None = 0;
 * inline const Type OwnerFinished = (1 << 4);
 * inline const Type InvalidPath = (1 << 5);
 * inline const Type MovementStop = (1 << 6);
 * inline const Type NewRequest = (1 << 7);
 * inline const Type ForcedScript = (1 << 8);
 * inline const Type AlreadyAtGoal = (1 << 9);
 */