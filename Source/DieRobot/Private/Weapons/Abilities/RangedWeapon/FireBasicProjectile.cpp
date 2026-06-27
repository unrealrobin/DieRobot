// Property of Paracosm.


#include "Weapons/Abilities/RangedWeapon/FireBasicProjectile.h"

#include "Components/Combat/CombatComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundCue.h"
#include "Weapons/TimberWeaponRangedBase.h"
#include "Weapons/Projectiles/TimberPlayerProjectile.h"

UFireBasicProjectile::UFireBasicProjectile()
{
	ABILITY_NAME = "FireBasicProjectile";
	ValidationType = EAbilityValidation::RequiresPower;
	WeaponAbilityType = EForWeaponType::RangedWeapon;
	bNeedsProjectileData = true;
}

void UFireBasicProjectile::Execute(FAbilityContext Context)
{
	//UE_LOG(LogTemp, Warning, TEXT("In Fire Ranged Weapon"));
	if (ProjectileClass)
	{
		if (ATimberWeaponRangedBase* OwningWeapon = Cast<ATimberWeaponRangedBase>(Context.WeaponInstance))
		{
			FVector ProjectileSpawnLocation = OwningWeapon->ProjectileSpawnComponent->GetComponentLocation();
			FRotator ProjectileAimRotation = (Context.TargetLocation - ProjectileSpawnLocation).Rotation();
			
			FActorSpawnParameters SpawnParams;
			SpawnParams.Owner = OwningWeapon;
			SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
			
			FTransform ProjectileSpawnTransform = FTransform(ProjectileAimRotation, ProjectileSpawnLocation);
			
			ATimberPlayerProjectile* Projectile = GetWorld()->SpawnActorDeferred<ATimberPlayerProjectile>(
				ProjectileClass,
				ProjectileSpawnTransform,
				SpawnParams.Owner);

			//Instigator is the Owner of the Combat Component
			Projectile->PlayerProjectileOwner = Cast<ATimberPlayableCharacter>(Context.Instigator);
			
			Projectile->FinishSpawning(ProjectileSpawnTransform);

			//After spawning, this Ability needs to lets the Weapon know that it was just fired and to initiate a projectile Spawn cooldown
			if (Projectile)
			{
				HandleMuzzleFlash(NiagaraEffect, ProjectileSpawnLocation, ProjectileAimRotation, true, true);
				UGameplayStatics::PlaySoundAtLocation(GetWorld(), OwningWeapon->FiringSound, ProjectileSpawnLocation);
				
				//The type of projectile controls the Firing Rate in this Case.
				OwningWeapon->HandleFiringRate(TimeBetweenProjectiles);

				//Controls the Ammo Cost of the Weapon and what happens at 0.
				OwningWeapon->ConsumePower(PowerCost);

				HandleCleanup(Context);
			}
		};
	}
}

void UFireBasicProjectile::HandleMuzzleFlash(
	UNiagaraSystem* NiagaraSystem, FVector Location, FRotator Rotation, bool AutoDestroy, bool AutoActive)
{
	PlayNiagaraEffectAtLocation(NiagaraSystem, Location, Rotation, AutoDestroy, AutoActive);
}
