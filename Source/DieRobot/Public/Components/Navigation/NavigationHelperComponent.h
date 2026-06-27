// Property of Paracosm.

#pragma once

#include "CoreMinimal.h"
#include "NavigationPath.h"
#include "Components/ActorComponent.h"
#include "NavigationHelperComponent.generated.h"

class ABuildableBase;
class ATimberCharacterBase;

UENUM(BlueprintType)
enum class EPathFailureReason : uint8
{
	None,
	NoPathExists,
	Blocked,
	TargetUnreachable
};

//Struct for Path Finding Result
USTRUCT(BlueprintType)
struct FPathResult
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Path Result")
	TArray<FVector> PathPoints;

	UPROPERTY(BlueprintReadOnly, Category = "Path Result")
	bool bIsValid = false;

	UPROPERTY(BlueprintReadOnly, Category = "Path Result")
	bool bIsPartial = false;

	UPROPERTY(BlueprintReadOnly, Category = "Path Result")
	FVector ClosestAccessiblePoint = FVector::ZeroVector;

	UPROPERTY(BlueprintReadOnly, Category = "Path Result")
	EPathFailureReason FailureReason = EPathFailureReason::None;
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class DIEROBOT_API UNavigationHelperComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	UNavigationHelperComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Navigation Data")
	AActor* OwningActor = nullptr;
	
	UFUNCTION()
	UNavigationPath* GetOriginalNavPath(FVector Start, FVector End);

public:

	//Path Calculations with Fallbacks
	UFUNCTION(BlueprintCallable, Category = "Navigation Logic")
	FPathResult CalculatePathToTarget(FVector Start, FVector End);

	//Find Closest Accessible point when no direct path exists.
	UFUNCTION(BlueprintCallable, Category = "Navigation Logic")
	FVector FindClosestAccessiblePoint(FVector Start, FVector Target);
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Navigation Data")
	bool bIsLastPathPartial = false;
	
	//Uses UE Nav Path and adjusts it to be centered on Polys.
	UFUNCTION(BlueprintCallable, Category = "Navigation Logic")
	TArray<FVector> GetCorridorPathPoints(FVector Start, FVector End);

	FVector GetCenterOfNode(NavNodeRef Node);
	FVector LastPathPoint = FVector::ZeroVector;
	bool CheckIfPathShouldUpdate(ABuildableBase* BuildableActor);
	
};
