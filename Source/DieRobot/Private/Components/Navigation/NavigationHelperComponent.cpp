// Property of Paracosm.


#include "Components/Navigation/NavigationHelperComponent.h"
#include "NavigationSystem.h"
#include "BuildSystem/BuildableBase.h"
#include "NavMesh/RecastNavMesh.h"


// Sets default values for this component's properties
UNavigationHelperComponent::UNavigationHelperComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

// Called when the game starts
void UNavigationHelperComponent::BeginPlay()
{
	Super::BeginPlay();

	OwningActor = GetOwner();
	if (!OwningActor)
	{
		//UE_LOG(LogTemp, Warning, TEXT("NavigationHelperComponent: Owning character is not set or invalid!"));
	}
}


UNavigationPath* UNavigationHelperComponent::GetOriginalNavPath(FVector Start, FVector End)
{
	UNavigationPath* NavPath = UNavigationSystemV1::FindPathToLocationSynchronously(GetWorld(), Start, End);

	if (!NavPath || !IsValid(NavPath) || NavPath->PathPoints.Num() == 0)
	{
		//UE_LOG(LogTemp, Warning, TEXT("NavigationHelperComponent: Failed to find a valid navigation path!"));
		return nullptr;
	}
	NavPath->IsPartial() ? bIsLastPathPartial = true : bIsLastPathPartial = false;
	return NavPath;
}

FPathResult UNavigationHelperComponent::CalculatePathToTarget(FVector Start, FVector End)
{
	FPathResult Result;

	// First attempt: Direct path
	UNavigationPath* NavPath = GetOriginalNavPath(Start, End);

	if (NavPath && NavPath->IsValid() && NavPath->PathPoints.Num() > 0)
	{
		Result.PathPoints = GetCorridorPathPoints(Start, End);
		Result.bIsValid = true;
		Result.bIsPartial = NavPath->IsPartial();

		if (Result.bIsPartial)
		{
			UE_LOG(LogTemp, Warning, TEXT("Partial path found - enemy will destroy blocking walls"));
		}
		return Result;
	}

	// Fallback: Find closest accessible point
	FVector ClosestPoint = FindClosestAccessiblePoint(Start, End);

	if (ClosestPoint != FVector::ZeroVector)
	{
		NavPath = GetOriginalNavPath(Start, ClosestPoint);
		if (NavPath && NavPath->IsValid())
		{
			Result.PathPoints = GetCorridorPathPoints(Start, ClosestPoint);
			Result.bIsValid = true;
			Result.bIsPartial = true;// Always partial since not reaching final target
			Result.ClosestAccessiblePoint = ClosestPoint;
			Result.FailureReason = EPathFailureReason::TargetUnreachable;

			//UE_LOG(LogTemp, Warning, TEXT("No direct path - moving to closest accessible point"));
			return Result;
		}
	}

	// Complete failure
	Result.bIsValid = false;
	Result.FailureReason = EPathFailureReason::NoPathExists;
	UE_LOG(LogTemp, Error, TEXT("No path possible to target"));

	return Result;
}

FVector UNavigationHelperComponent::FindClosestAccessiblePoint(FVector Start, FVector Target)
{
	// Search in expanding radius from target
	float SearchRadius = 200.0f;
	const float MaxSearchRadius = 2000.0f;
	const float RadiusIncrement = 100.0f;

	while (SearchRadius <= MaxSearchRadius)
	{
		// Test points in a circle around the target
		const int32 NumTestPoints = 12;
		for (int32 i = 0; i < NumTestPoints; i++)
		{
			float Angle = (2.0f * PI * i) / NumTestPoints;
			FVector TestPoint = Target + FVector(
				FMath::Cos(Angle) * SearchRadius,
				FMath::Sin(Angle) * SearchRadius,
				0.0f);

			// Check if this point is accessible
			UNavigationPath* TestPath = GetOriginalNavPath(Start, TestPoint);
			if (TestPath && TestPath->IsValid() && !TestPath->IsPartial())
			{
				return TestPoint;
			}
		}
		SearchRadius += RadiusIncrement;
	}
	return FVector::ZeroVector;// No accessible point found
}

FVector UNavigationHelperComponent::GetCenterOfNode(NavNodeRef Node)
{
	//This is the Navigation System the Owning Character is using.
	UNavigationSystemV1* NavSys = UNavigationSystemV1::GetCurrent(OwningActor->GetWorld());
	ARecastNavMesh* RecastMesh = Cast<ARecastNavMesh>(NavSys->GetDefaultNavDataInstance());
	
	TArray<FVector> Verts;
	
	RecastMesh->GetPolyVerts(Node, Verts );

	FVector SumOfVerts = FVector::ZeroVector;
	for (FVector Vert : Verts)
	{
		SumOfVerts += Vert;
	}
	FVector CenterOfNode = SumOfVerts / Verts.Num();
	
	return CenterOfNode;
}

TArray<FVector> UNavigationHelperComponent::GetCorridorPathPoints(FVector Start, FVector End)
{
	TArray<FVector> PathPoints;
	
	//DEBUG USE ONLY
	if (!OwningActor || !OwningActor->GetWorld()) return PathPoints;
	
	UNavigationPath* NavPath = GetOriginalNavPath(Start, End);
	
	// Retrieving the real object with all the Low Level Data
	if (NavPath && IsValid(NavPath))
	{
		//Debug to show the original Path Points in the world.
		/*for (FVector OriginalPathPoint : NavPath->PathPoints)
		{
			DrawDebugSphere(GetWorld(), OriginalPathPoint, 10, 10, FColor::Red, true);
		}*/
		
		FNavPathSharedPtr NavPathSharedPtr = NavPath->GetPath();
		FNavMeshPath*  MeshPath = static_cast<FNavMeshPath*>(NavPathSharedPtr.Get());

		if (NavPath->PathPoints.Num()  == 0 || !MeshPath)
		{
			UE_LOG(LogTemp, Warning, TEXT("UNavigationHelperComponent: Failed to retrieve the NavMesh Path!"));
			return PathPoints;
		}

		//Nav Mesh Poly Ids for Corridor Path - The Path Points lie within these polys, All the polys are connected and form a corridor.
		//We now have access to those Polys IDs
		const TArray<NavNodeRef>& Corridor = MeshPath->PathCorridor;
		
		//This is the Navigation System the Owning Character is using.
		UNavigationSystemV1* NavSys = UNavigationSystemV1::GetCurrent(OwningActor->GetWorld());

		ARecastNavMesh* RecastMesh = Cast<ARecastNavMesh>(NavSys->GetDefaultNavDataInstance());

		if (!RecastMesh) return PathPoints;
		
		//Looping as long as the Iterations are less than the number of Corridor Points.
		// We iterate while we can still access the next Poly ID in the Corridor array.
		for (int32 i = 0; i + 1 < Corridor.Num(); ++i)
		{
			NavNodeRef PolyID_A = Corridor[i];
			TArray<FVector> PolyVerts_A;
			
			NavNodeRef PolyID_B = Corridor[i + 1];
			TArray<FVector> PolyVerts_B;

			FVector PortalVector_A = FVector::ZeroVector;
			FVector PortalVector_B = FVector::ZeroVector;

			//Getting all the Verts for Each Poly
			RecastMesh->GetPolyVerts(PolyID_A, PolyVerts_A);
			RecastMesh->GetPolyVerts(PolyID_B, PolyVerts_B);

			//finding the 2 Portal vectors that the 2 polys Share
			for (FVector Vert : PolyVerts_A)
			{
				if (PolyVerts_B.Contains(Vert))
				{
					PortalVector_A == FVector::ZeroVector ? PortalVector_A = Vert : PortalVector_B = Vert;

					if (PortalVector_B != FVector::ZeroVector)
					{
						// We have both Portal Vectors, we can break the loop.
						break;
					}
				}
			}

			//Getting the Approx Center of the Shared Edge.
			FVector CenteredPortalVector = (PortalVector_A + PortalVector_B) / 2;

			//Adding the Centered Portal Vector to the Path Points Array. This will serve as the adjusted Path Point.
			PathPoints.Add(CenteredPortalVector);
		}

		//Adding the actual Specified End Location to the Path Points. This is an unadjusted point.
		LastPathPoint = NavPath->PathPoints.Last();
		
		PathPoints.Add(LastPathPoint);
		
		//Debug to show the adjusted Path Points in the world.
		/*for (FVector PathPoint : PathPoints)
		{
			DrawDebugSphere(GetWorld(), PathPoint, 10, 10, FColor::Green, false, 10.0f);
		}*/
	}
	return PathPoints;
}

bool UNavigationHelperComponent::CheckIfPathShouldUpdate(ABuildableBase* BuildableActor)
{
	if (BuildableActor)
	{
		//TODO:: Add some Var to ABuildableBase to Check whether this Buildable effects Navigation / Can block or alter paths.
		if (BuildableActor->BuildableType == EBuildableType::BuildingComponent || BuildableActor->BuildableType == EBuildableType::Ramp)
		{
			return true;
		}
		return false;
	}
	UE_LOG(LogTemp, Warning, TEXT("Buildable Actor is Not Valid."))
	return false;
}

