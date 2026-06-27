// Property of Paracosm Industries. Dont use my shit.


#include "BuildSystem/BuildingComponents/DieRobotHorizontalBuildingComponent.h"


// Sets default values
ADieRobotHorizontalBuildingComponent::ADieRobotHorizontalBuildingComponent()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	BuildingOrientation = EBuildingComponentOrientation::Horizontal;
	SnapCondition = ESnapCondition::BuildingComponent;
	BuildableType = EBuildableType::BuildingComponent;
}

// Called when the game starts or when spawned
void ADieRobotHorizontalBuildingComponent::BeginPlay()
{
	Super::BeginPlay();
}

// Called every frame
void ADieRobotHorizontalBuildingComponent::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}
