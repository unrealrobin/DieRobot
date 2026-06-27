// Property of Paracosm Industries. Dont use my shit.


#include "BuildSystem/BuildingComponents/TimberHorizontalBuildingComponent.h"


// Sets default values
ATimberHorizontalBuildingComponent::ATimberHorizontalBuildingComponent()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	BuildingOrientation = EBuildingComponentOrientation::Horizontal;
	SnapCondition = ESnapCondition::BuildingComponent;
	BuildableType = EBuildableType::BuildingComponent;
}

// Called when the game starts or when spawned
void ATimberHorizontalBuildingComponent::BeginPlay()
{
	Super::BeginPlay();
}

// Called every frame
void ATimberHorizontalBuildingComponent::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}
