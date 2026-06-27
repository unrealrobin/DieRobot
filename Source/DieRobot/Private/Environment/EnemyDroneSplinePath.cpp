// Property of Paracosm.


#include "Environment/EnemyDroneSplinePath.h"

#include "Components/SplineComponent.h"


// Sets default values
AEnemyDroneSplinePath::AEnemyDroneSplinePath()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	RootSceneComponent = CreateDefaultSubobject<USceneComponent>("RootSceneComponent");
	RootComponent = RootSceneComponent;
	
	SplineComponent = CreateDefaultSubobject<USplineComponent>("Spline");
	if (RootComponent)
	{
		SplineComponent->SetupAttachment(RootComponent);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("No Root Component for SplineComponent to attach to!"));
	}
}

// Called when the game starts or when spawned
void AEnemyDroneSplinePath::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AEnemyDroneSplinePath::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

