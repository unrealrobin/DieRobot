// Property of Paracosm.

#pragma once

#include "CoreMinimal.h"
#include "Weapons/Abilities/WeaponAbilityBase.h"
#include "FireBasicProjectile.generated.h"

class ADieRobotProjectileBase;
/**
 * 
 */
UCLASS(Blueprintable)
class DIEROBOT_API UFireBasicProjectile : public UWeaponAbilityBase
{
	GENERATED_BODY()

public:
	
	UFireBasicProjectile();
	
	virtual void Execute(FAbilityContext Context) override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ability Details")
	float TimeBetweenProjectiles = 0.5f;

protected:
	

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Projectile")
	TSubclassOf<ADieRobotProjectileBase> ProjectileClass;

	void HandleMuzzleFlash(UNiagaraSystem* NiagaraSystem, FVector Location, FRotator Rotation = FRotator::ZeroRotator, bool AutoDestroy = true, bool AutoActive = true);
	
};
