// Property of Paracosm Industries. Dont use my shit.

#pragma once

#include "CoreMinimal.h"
#include "TimberBuildingComponentBase.h"
#include "TimberVerticalBuildingComponent.generated.h"

UCLASS()
class DIEROBOT_API ATimberVerticalBuildingComponent : public ATimberBuildingComponentBase
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	ATimberVerticalBuildingComponent();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;
};
