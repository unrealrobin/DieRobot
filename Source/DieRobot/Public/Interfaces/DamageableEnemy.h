// Property of Paracosm Industries.

#pragma once

#include "CoreMinimal.h"
#include "Types/Combat/DamagePayload.h"
#include "UObject/Interface.h"
#include "DamageableEnemy.generated.h"

// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class UDamageableEnemy : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class DIEROBOT_API IDamageableEnemy
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:
	virtual void TakeDamage(FDamagePayload DamagePayload) = 0;
	virtual void PlayProjectileHitSound(FHitResult HitResult) = 0;
	virtual void PlayMeleeWeaponHitSound(FHitResult HitResult) = 0;
};
