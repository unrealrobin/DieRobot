// Property of Paracosm Industries. Dont use my shit.


#include "BuildSystem/Traps/PlusTrapBase.h"

#include "BuildSystem/BuildingComponents/TimberBuildingComponentBase.h"
#include "Components/BoxComponent.h"



// Sets default values
APlusTrapBase::APlusTrapBase()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	BoxExtentRaycastStart = CreateDefaultSubobject<USceneComponent>("RaycastStart");
	BoxExtentRaycastStart->SetupAttachment(RootComponent);
}

// Called when the game starts or when spawned
void APlusTrapBase::BeginPlay()
{
	Super::BeginPlay();

	RaycastForHitBoxLength();
	
	FTimerHandle RaycastForBoxLengthTimerHandle;
	GetWorld()->GetTimerManager().SetTimer(RaycastForBoxLengthTimerHandle, this, &APlusTrapBase::RaycastForHitBoxLength, 0.3f, true);
}

void APlusTrapBase::RaycastForHitBoxLength()
{
	//Only Dynamically adjusts the HitBoxLength
	
	//Raycast Start and End Points
	FVector StartPoint = BoxExtentRaycastStart->GetComponentLocation();

	//Max Hit Box Length is the furthest we want the extend of this Plus Trap to fire.
	FVector EndPoint = StartPoint + ( BoxExtentRaycastStart->GetForwardVector() * MaxHitBoxLength);
	
	bool bHits = GetWorld()->LineTraceMultiByChannel(HitResults, StartPoint, EndPoint, ECC_Visibility );

	
	//We default this to EndPoint for in the case there is no hit, The Max Distance we want the HitBox to be is the MaxHitBoxLength
	FVector HitLocation = EndPoint;
	
	if (bHits)
	{
		for (FHitResult Hit : HitResults)
		{
			/*
			 * 1. Raycast Until we hit a BuildableBase
			 * 2. Ensuring Hit Actor is NOT the TrapHoveredBuildingComponent (this is this First Hit Buildable in the Player Raycast Hit Results)
			 * 3. Ignore Self
			 */
			//UE_LOG(LogTemp, Warning, TEXT("Plus Trap Hit Box Length Hit: %s ,  Hit Component: %s"), *Hit.GetActor()->GetName(),*Hit.GetComponent()->GetName());
			 if (Cast<ABuildableBase>(Hit.GetActor()) && Cast<ABuildableBase>(Hit.GetActor()) != TrapHoveredBuildingComponent && Hit.GetActor() != this) 
			 {
				HitLocation = Hit.ImpactPoint;
			 	//Debug the Hit Point
			 	/*DrawDebugSphere(GetWorld(), HitLocation, 10.f, 12, FColor::Blue, false, -1.f);
			 	DrawDebugSphere(GetWorld(), EndPoint, 10.f, 12, FColor::Red, false, -1.f);
			 	DrawDebugSphere(GetWorld(), StartPoint, 10.f, 12, FColor::Green, false, -1.f);
			 	DrawDebugLine(GetWorld(), StartPoint, EndPoint, FColor::Red, false, -1.f, 0, 2.f);*/
			 	break;
			 }
		}
	}
	else
	{
		//UE_LOG(LogTemp, Warning, TEXT("No Hit"));
		/*DrawDebugSphere(GetWorld(), HitLocation, 10.f, 12, FColor::Blue, false, -1.f);
		DrawDebugSphere(GetWorld(), EndPoint, 10.f, 12, FColor::Red, false, -1.f);
		DrawDebugSphere(GetWorld(), StartPoint, 10.f, 12, FColor::Green, false, -1.f);
		DrawDebugLine(GetWorld(), StartPoint, EndPoint, FColor::Red, false, -1.f, 0, 2.f);*/
	}

	//The difference between the Raycast Start and End. Identifies the length at the blocking Hit.
	float DistanceDifference = FVector::Dist(HitLocation, StartPoint);

	//To be used for Trap Overlap Elements. Ex. This is the Distance a Pulse Box travels with Pulse Box Trap
	CalculatedBoxLength = DistanceDifference;

	//Resetting the Actual Box Length after a Raycast
	BoxLength = DistanceDifference / 2 + 30.f;

	//Setting the Box Extent Vector
	FVector DynamicBoxExtent = FVector(BoxWidth, BoxLength, BoxHeight);

	//Setting the Box Extent
	HitBoxComponent->SetBoxExtent(DynamicBoxExtent);

	HitBoxComponent->SetCollisionProfileName("DR_HitEventOnly");
	HitBoxComponent->bDynamicObstacle = true;

	//Relative Locations because when scaling extents the box components scales in 2 directions
	// -y <--MHBL--> +y
	//So we position the center of the Dynamic HitBox in the middle of the Extent Scale
	// So if we Want to scale the box to be 800 units long, we need to set the relative location to be 400 units in the Y direction (400 In each Direction)
	// We use 30 Units to account for the Offset of the Raycast Start point from the Actual Trap Center (+- 30/40)
	HitBoxComponent->SetRelativeLocation(FVector(0.f, BoxLength, 0.f));
}

// Called every frame
void APlusTrapBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	RaycastForHitBoxLength();
	
}

