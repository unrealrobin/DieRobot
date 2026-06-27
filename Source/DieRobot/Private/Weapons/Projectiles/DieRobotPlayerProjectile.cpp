// Property of Paracosm Industries. Dont use my shit.
#include "Weapons/Projectiles/DieRobotPlayerProjectile.h"

#include "Components/CapsuleComponent.h"
#include "Interfaces/DamageableEnemy.h"
#include "Types/Combat/DamagePayload.h"
#include "Weapons/DieRobotWeaponRangedBase.h"

class IDamageableEnemy;
// Sets default values
ADieRobotPlayerProjectile::ADieRobotPlayerProjectile()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	//Overlap Delegate
	if (CapsuleComponent)
	{
		//CapsuleComponent->OnComponentBeginOverlap.AddDynamic(this, &ADieRobotPlayerProjectile::HandleOverlap);
		CapsuleComponent->OnComponentHit.AddDynamic(this, &ADieRobotPlayerProjectile::HandleBlocked);
	}
}

// Called when the game starts or when spawned
void ADieRobotPlayerProjectile::BeginPlay()
{
	Super::BeginPlay();
	//UE_LOG(LogTemp, Warning, TEXT("Projectile Spawned."));
}

// Called every frame
void ADieRobotPlayerProjectile::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void ADieRobotPlayerProjectile::HandleBlocked(
	UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse,
	const FHitResult& Hit)
{
	
	IDamageableEnemy* HitEnemy = Cast<IDamageableEnemy>(OtherActor);

	if (HitEnemy)
	{
		//UE_LOG(LogTemp, Warning, TEXT("HitEnemy Valid."));
		//Play the IDamageableEnemy's TakeDamage function. Interface.
		HitEnemy->PlayProjectileHitSound(Hit);

		//Weapon Owns Projectile, Player Owns Weapon.
		if (PlayerProjectileOwner)
		{
			//UE_LOG(LogTemp, Warning, TEXT("Owning Weapon is Valid."));
			//HitEnemy->TakeDamage(CalculateOutputDamage(Cast<ADieRobotWeaponRangedBase>(GetOwner())), PlayerProjectileOwner, TODO);
			FDamagePayload Payload;
			Payload.DamageAmount = CalculateOutputDamage(Cast<ADieRobotWeaponRangedBase>(GetOwner()));
			Payload.DamageInstigator = PlayerProjectileOwner;
			HitEnemy->TakeDamage(Payload);
		}
	}
	
	GetWorld()->GetTimerManager().SetTimerForNextTick(this, &ADieRobotPlayerProjectile::HandleDestroy);
}

float ADieRobotPlayerProjectile::CalculateOutputDamage(ADieRobotWeaponRangedBase* Weapon)
{
	/*Projectile Damage can be modified by the Players Modifier && The Weapons Modifier
	 * This will allow effects to either Modify only Ranged Weapons or Player and Ranged Weapons or just the Player.
	 */
	if (PlayerProjectileOwner)
	{
		float TotalDamage = ProjectileBaseDamage * (Weapon->DamageModifierValue) * (PlayerProjectileOwner->DamageModifierValue);
		
		/*UE_LOG(LogTemp, Warning, TEXT("BaseProjectileDamage: %f. WeaponModifierValue: %f. PlayerModifierValue: %f.  Total Damage: %f"),
			ProjectileBaseDamage,
			Weapon->DamageModifierValue,
			PlayerProjectileOwner->DamageModifierValue,
			TotalDamage);*/
		
		return TotalDamage;
	}
	return ProjectileBaseDamage;
}

void ADieRobotPlayerProjectile::HandleDestroy()
{
	//UE_LOG(LogTemp, Warning, TEXT("Projectile Destroyed!"));
	Destroy();
}