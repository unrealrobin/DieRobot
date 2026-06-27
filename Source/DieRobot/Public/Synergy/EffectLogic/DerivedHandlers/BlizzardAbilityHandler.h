// Property of Paracosm.

#pragma once

#include "CoreMinimal.h"
#include "Synergy/EffectLogic/EffectAbilityHandlerBase.h"
#include "BlizzardAbilityHandler.generated.h"

/**
 * 
 */
UCLASS()
class DIEROBOT_API UBlizzardAbilityHandler : public UEffectAbilityHandlerBase
{
	GENERATED_BODY()

	UBlizzardAbilityHandler();
	
	void ExecuteEffect(AActor* TargetActor, const FStatusEffect& StatusEffect) override;
};
