// Property of Paracosm.

#pragma once

#include "CoreMinimal.h"

#include "EnemyDroneSplinePath.generated.h"

class USplineComponent;

UCLASS()
class DIEROBOT_API AEnemyDroneSplinePath : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	AEnemyDroneSplinePath();
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spline Path")
	USceneComponent* RootSceneComponent;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spline Path")
	USplineComponent* SplineComponent;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;
};
