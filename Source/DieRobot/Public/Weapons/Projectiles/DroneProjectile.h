// Property of Paracosm.

#pragma once

#include "CoreMinimal.h"
#include "DieRobotEnemyProjectile.h"
#include "DroneProjectile.generated.h"

UCLASS()
class DIEROBOT_API ADroneProjectile : public ADieRobotEnemyProjectile
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	ADroneProjectile();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	virtual bool IsActorCurrentTarget(AActor* OtherActor) override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;
};
