// Property of Paracosm Industries.


#include "Weapons/Projectiles/DieRobotEnemyProjectile.h"
#include "BuildSystem/BuildingComponents/DieRobotBuildingComponentBase.h"
#include "Character/DieRobotSeeda.h"
#include "Character/Enemies/DieRobotEnemyCharacter.h"
#include "Components/CapsuleComponent.h"
#include "Weapons/DieRobotWeaponRangedBase.h"


// Sets default values
ADieRobotEnemyProjectile::ADieRobotEnemyProjectile()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	if (CapsuleComponent)
	{
		CapsuleComponent->OnComponentBeginOverlap.AddDynamic(this, &ADieRobotEnemyProjectile::HandleOverlap);
		CapsuleComponent->OnComponentHit.AddDynamic(this, &ADieRobotEnemyProjectile::HandleBlocked);
	}
}

// Called when the game starts or when spawned
void ADieRobotEnemyProjectile::BeginPlay()
{
	Super::BeginPlay();

	//UE_LOG(LogTemp, Warning, TEXT("Enemy Projectile Spawned."));
}

// Called every frame
void ADieRobotEnemyProjectile::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void ADieRobotEnemyProjectile::HandleBlocked(
	UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse,
	const FHitResult& Hit)
{
	//If the Other Actor isn't the current Target, do not overlap Damage.
	if (!IsActorCurrentTarget(OtherActor))
	{
		Destroy();
		return;
	}
	
	ADieRobotBuildingComponentBase* BuildingComponent = Cast<ADieRobotBuildingComponentBase>(OtherActor);
	if (BuildingComponent && BuildingComponent->BuildingComponentType != EBuildingComponentType::Environment)
	{
		//If the projectile is blocked by a BuildingComponentBase (Wall, Floor, Ramp), damage the BuildingComponent.
		//Traps and constructs don't take damage from enemy projectiles yet.
		BuildingComponent->BuildingComponentTakeDamage(ProjectileBaseDamage, this);
		//UE_LOG(LogTemp, Warning, TEXT("EnemyProjectile - Enemy Projectile has been Blocked and Damaged Building Component."))
		Destroy();
	}
	else
	{
		//Blocked by a non-damageable object. Destroy the projectile.
		Destroy();
		//UE_LOG(LogTemp, Warning, TEXT("EnemyProjectile - Enemy Projectile has been Blocked and Destroyed - No Damage."))
	}
}

void ADieRobotEnemyProjectile::HandleOverlap(
	UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
	bool bFromSweep, const FHitResult& SweepResult)
{
	//If the Other Actor isn't the current Target, do not overlap Damage.
	if (!IsActorCurrentTarget(OtherActor))
	{
		//Destroy();
		return;
	}
	
	ADieRobotPlayableCharacter* PlayerCharacter = Cast<ADieRobotPlayableCharacter>(OtherActor);
	ADieRobotSeeda* Seeda = Cast<ADieRobotSeeda>(OtherActor);

	//Enemy Projectile Damage is Output to the Player Character or Seeda's Take damage functions -
	//Enemy Damage Reduction Principles are applied in their own functions.

	//UE_LOG(LogTemp, Warning, TEXT("Enemy Projectile Overlap Detected. Overlapped: %s"), *OtherComp->GetName());
	
	if (PlayerCharacter)
	{
		PlayerCharacter->PlayerTakeDamage(ProjectileBaseDamage);
		Destroy();
	}

	if(Seeda)
	{
		Seeda->TakeDamage_Seeda(ProjectileBaseDamage);
		Destroy();
	}

	//UE_LOG(LogTemp, Warning, TEXT("Enemy Projectile Destroyed on Overlap."))
	
}

bool ADieRobotEnemyProjectile::IsActorCurrentTarget(AActor* OtherActor)
{
	//Double Get Owner because most projectiles fire from Gun, but we need GunOwner
	// Projectile -> Gun -> GunOwner
	if (ADieRobotWeaponRangedBase* OwningWeapon = Cast<ADieRobotWeaponRangedBase>(GetOwner()))
	{
		if (ADieRobotEnemyCharacter* OwningEnemyCharacter = Cast<ADieRobotEnemyCharacter>(OwningWeapon->GetOwner()))
		{
			if (OtherActor == OwningEnemyCharacter->CurrentTarget)
			{
				return true;
			}
		}
	}

	return false;
}
