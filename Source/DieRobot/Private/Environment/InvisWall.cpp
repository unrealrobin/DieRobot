// Property of Paracosm Industries. Dont use my shit.


#include "Environment/InvisWall.h"


// Sets default values
AInvisWall::AInvisWall()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	RootSceneComponent = CreateDefaultSubobject<USceneComponent>("RootSceneComponent");
	RootComponent = RootSceneComponent;
	WallStaticMesh = CreateDefaultSubobject<UStaticMeshComponent>("StaticMesh");
	WallStaticMesh->SetupAttachment(RootComponent);
}

// Called when the game starts or when spawned
void AInvisWall::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AInvisWall::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

