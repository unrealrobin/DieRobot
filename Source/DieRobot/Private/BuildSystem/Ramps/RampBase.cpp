// Property of Paracosm Industries. 


#include "BuildSystem/Ramps/RampBase.h"

#include "Character/TimberSeeda.h"
#include "Components/BoxComponent.h"


// Sets default values
ARampBase::ARampBase()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	BuildableType = EBuildableType::Ramp;
	SnapCondition = ESnapCondition::Ramp;
	
	StaticMesh = CreateDefaultSubobject<UStaticMeshComponent>("StaticMesh");
	StaticMesh->SetupAttachment(RootComponent);
	VerticalCenterSnap = CreateDefaultSubobject<USceneComponent>("VerticalCenterSnap");
	HorizontalCenterSnap = CreateDefaultSubobject<USceneComponent>("HorizontalCenterSnap");
	VerticalCenterSnap->SetupAttachment(RootComponent);
	HorizontalCenterSnap->SetupAttachment(RootComponent);
	CenterRampLocation = CreateDefaultSubobject<USceneComponent>("CenterRampLocation");
	CenterRampLocation->SetupAttachment(RootComponent);
}

// Called when the game starts or when spawned
void ARampBase::BeginPlay()
{
	Super::BeginPlay();
}

bool ARampBase::bIsRampProxyBlocked()
{
	//Create a bounding Box around the center of the Ramp that checks for whether Seeda or another ramp is in the way.
	FCollisionShape CollisionCapsule = FCollisionShape::MakeCapsule(100.f, 100.f);
	TArray<FHitResult> Hits;
	FCollisionObjectQueryParams CollisionObjectQueryParams;

	//Tracing against these 2 channels.
	CollisionObjectQueryParams.AddObjectTypesToQuery(ECC_GameTraceChannel1);
	CollisionObjectQueryParams.AddObjectTypesToQuery(ECC_Pawn);
	
	bool bHits = GetWorld()->SweepMultiByObjectType(Hits, 
		CenterRampLocation->GetComponentLocation(), 
		CenterRampLocation->GetComponentLocation(), 
		FQuat::Identity, 
		CollisionObjectQueryParams, 
		CollisionCapsule);

	if (bHits)
	{
		for (FHitResult IndividualHit : Hits)
		{
			//Ignoring Self 
			if (IndividualHit.GetActor() == this) continue;
			
			if (Cast<ATimberSeeda>(IndividualHit.GetActor()) || Cast<ARampBase>(IndividualHit.GetActor()) || Cast<ABuildableBase>(IndividualHit.GetActor()))
			{
				UE_LOG(LogTemp, Display, TEXT("Ramp Blocked by: %s"), *IndividualHit.GetActor()->GetName());
				return true;
			}
		}
	}
	
	return false;
}

// Called every frame
void ARampBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}
