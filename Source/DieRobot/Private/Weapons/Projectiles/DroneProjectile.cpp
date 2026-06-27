// Property of Paracosm.


#include "Weapons/Projectiles/DroneProjectile.h"

#include "Character/Enemies/TimberEnemyCharacter.h"


class ATimberEnemyCharacter;
// Sets default values
ADroneProjectile::ADroneProjectile()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}

// Called when the game starts or when spawned
void ADroneProjectile::BeginPlay()
{
	Super::BeginPlay();
	
}

bool ADroneProjectile::IsActorCurrentTarget(AActor* OtherActor)
{
	//Used for Drones
	// Projectile->Drone Enemy
	if (ATimberEnemyCharacter* EnemyCharacter = Cast<ATimberEnemyCharacter>(GetOwner()))
	{
		if (OtherActor == EnemyCharacter->CurrentTarget)
		{
			return true;
		}

		//There can potentially be No Current Target for Drones if there are no Buildables on the Map.
		//TODO:: If no Buildables found during Drone GatherTargets, then set the CurrentTarget to the Player Character and force the Drone to attack the Player.
		if (EnemyCharacter && EnemyCharacter->CurrentTarget)
		{
			UE_LOG(LogTemp, Warning, TEXT("Drone Current Target: %s"), *EnemyCharacter->CurrentTarget->GetName());
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Owner of Drone Projectile is not a Timber Enemy Character!"));
	}
	return false;
}

// Called every frame
void ADroneProjectile::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

