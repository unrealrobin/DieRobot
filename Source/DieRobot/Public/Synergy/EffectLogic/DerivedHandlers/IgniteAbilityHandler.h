// Property of Paracosm.

#pragma once

#include "CoreMinimal.h"
#include "Synergy/EffectLogic/EffectAbilityHandlerBase.h"
#include "IgniteAbilityHandler.generated.h"

/**
 * 
 */
UCLASS()
class DIEROBOT_API UIgniteAbilityHandler : public UEffectAbilityHandlerBase
{
	GENERATED_BODY()

public:
	UIgniteAbilityHandler();
	
	void ExecuteEffect(AActor* TargetActor, const FStatusEffect& StatusEffect) override;
};
