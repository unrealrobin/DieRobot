// Property of Paracosm Industries. Dont use my shit.

#pragma once

#include "CoreMinimal.h"
#include "DieRobotBuildingComponentBase.h"
#include "DieRobotHorizontalBuildingComponent.generated.h"

UCLASS()
class DIEROBOT_API ADieRobotHorizontalBuildingComponent : public ADieRobotBuildingComponentBase
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	ADieRobotHorizontalBuildingComponent();


protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;
};
