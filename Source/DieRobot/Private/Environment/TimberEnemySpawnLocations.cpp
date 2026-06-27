// Property of Paracosm Industries. Dont use my shit.


#include "Environment/TimberEnemySpawnLocations.h"

// Sets default values
ATimberEnemySpawnLocations::ATimberEnemySpawnLocations()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	StaticMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>("StaticMesh");
	StaticMeshComponent->SetupAttachment(RootComponent);
	
}

// Called when the game starts or when spawned
void ATimberEnemySpawnLocations::BeginPlay()
{
	Super::BeginPlay();

	SpawnLocation = GetActorLocation();
	StaticMeshComponent->SetCollisionResponseToAllChannels(ECR_Ignore);
}
