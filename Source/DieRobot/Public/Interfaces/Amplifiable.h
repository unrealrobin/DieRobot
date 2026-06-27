// Property of Paracosm.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "Amplifiable.generated.h"

// This class does not need to be modified.
UINTERFACE()
class UAmplifiable : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class DIEROBOT_API IAmplifiable
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:

	//Each Interface Implementor handles how to react to being Amplified.
	virtual void SetIsAmplified(bool bIsAmplified) = 0;
	
};
