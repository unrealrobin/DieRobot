// Property of Paracosm Industries. Dont use my shit.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "Interactable.generated.h"

// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class UInteractable : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class DIEROBOT_API IInteractable
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:
	virtual void Interact() = 0;
};
