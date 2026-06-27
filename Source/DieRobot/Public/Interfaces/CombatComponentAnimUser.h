// Property of Paracosm.

#pragma once

#include "CoreMinimal.h"

#include "CombatComponentAnimUser.generated.h"

// This class does not need to be modified.
UINTERFACE()
class UCombatComponentAnimUser : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class DIEROBOT_API ICombatComponentAnimUser
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:

	virtual void PlayWeaponEquipAnimationMontage(FName SectionName) = 0;
};
