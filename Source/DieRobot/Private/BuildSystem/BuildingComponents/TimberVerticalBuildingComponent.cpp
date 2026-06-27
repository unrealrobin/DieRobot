// Property of Paracosm Industries. Dont use my shit.


#include "BuildSystem/buildingComponents/TimberVerticalBuildingComponent.h"


// Sets default values
ATimberVerticalBuildingComponent::ATimberVerticalBuildingComponent()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	BuildingOrientation = EBuildingComponentOrientation::Vertical;
	SnapCondition = ESnapCondition::BuildingComponent;
	BuildableType = EBuildableType::BuildingComponent;
}

// Called when the game starts or when spawned
void ATimberVerticalBuildingComponent::BeginPlay()
{
	Super::BeginPlay();
}

// Called every frame
void ATimberVerticalBuildingComponent::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}
