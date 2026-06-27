// Property of Paracosm Industries. Dont use my shit.


#include "BuildSystem/buildingComponents/DieRobotVerticalBuildingComponent.h"


// Sets default values
ADieRobotVerticalBuildingComponent::ADieRobotVerticalBuildingComponent()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	BuildingOrientation = EBuildingComponentOrientation::Vertical;
	SnapCondition = ESnapCondition::BuildingComponent;
	BuildableType = EBuildableType::BuildingComponent;
}

// Called when the game starts or when spawned
void ADieRobotVerticalBuildingComponent::BeginPlay()
{
	Super::BeginPlay();
}

// Called every frame
void ADieRobotVerticalBuildingComponent::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}
