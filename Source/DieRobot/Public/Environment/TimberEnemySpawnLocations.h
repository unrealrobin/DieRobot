// Property of Paracosm Industries. Dont use my shit.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "TimberEnemySpawnLocations.generated.h"

UCLASS()
class DIEROBOT_API ATimberEnemySpawnLocations : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	ATimberEnemySpawnLocations();

	FVector SpawnLocation = FVector(0.0f, 0.0f, 0.0f);

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UPROPERTY(EditAnywhere)
	UStaticMeshComponent* StaticMeshComponent;
};
