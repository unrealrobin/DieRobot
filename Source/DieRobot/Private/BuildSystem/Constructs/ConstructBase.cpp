// Property of Paracosm Industries.


#include "BuildSystem/Constructs/ConstructBase.h"


// Sets default values
AConstructBase::AConstructBase()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}

// Called when the game starts or when spawned
void AConstructBase::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AConstructBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

