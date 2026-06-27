// Property of Paracosm Industries. Dont use my shit.

#pragma once

#include "CoreMinimal.h"
#include "DieRobotBuildingComponentBase.h"
#include "DieRobotVerticalBuildingComponent.generated.h"

UCLASS()
class DIEROBOT_API ADieRobotVerticalBuildingComponent : public ADieRobotBuildingComponentBase
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	ADieRobotVerticalBuildingComponent();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;
};
