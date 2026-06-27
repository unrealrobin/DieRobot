// Property of Paracosm.

#pragma once

#include "CoreMinimal.h"
#include "Synergy/EffectLogic/EffectAbilityHandlerBase.h"
#include "ObliterateAbilityHandler.generated.h"

/**
 * 
 */
UCLASS()
class DIEROBOT_API UObliterateAbilityHandler : public UEffectAbilityHandlerBase
{
	GENERATED_BODY()
	
public:

	UObliterateAbilityHandler();
	
	void ExecuteEffect(AActor* TargetActor, const FStatusEffect& StatusEffect) override;
};
