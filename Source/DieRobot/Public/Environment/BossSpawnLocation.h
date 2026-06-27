// Property of Paracosm Industries. Dont use my shit.

#pragma once

#include "CoreMinimal.h"
#include "DieRobotEnemySpawnLocations.h"
#include "BossSpawnLocation.generated.h"

UCLASS()
class DIEROBOT_API ABossSpawnLocation : public AActor 
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	ABossSpawnLocation();
	
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	

public:
	// Called when the game starts or when spawned
	FVector SpawnLocation = FVector(0.0f, 0.0f, 0.0f);

	UPROPERTY(EditAnywhere)
	UStaticMeshComponent* StaticMeshComponent;
};
