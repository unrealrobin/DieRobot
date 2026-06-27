// Property of Paracosm Industries. Dont use my shit.


#include "Environment/DieRobotEnemySpawnLocations.h"

// Sets default values
ADieRobotEnemySpawnLocations::ADieRobotEnemySpawnLocations()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	StaticMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>("StaticMesh");
	StaticMeshComponent->SetupAttachment(RootComponent);
	
}

// Called when the game starts or when spawned
void ADieRobotEnemySpawnLocations::BeginPlay()
{
	Super::BeginPlay();

	SpawnLocation = GetActorLocation();
	StaticMeshComponent->SetCollisionResponseToAllChannels(ECR_Ignore);
}
