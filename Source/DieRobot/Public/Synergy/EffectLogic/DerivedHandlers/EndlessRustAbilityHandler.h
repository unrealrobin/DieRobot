// Property of Paracosm.

#pragma once

#include "CoreMinimal.h"
#include "Synergy/EffectLogic/EffectAbilityHandlerBase.h"
#include "EndlessRustAbilityHandler.generated.h"

/**
 * 
 */
UCLASS()
class DIEROBOT_API UEndlessRustAbilityHandler : public UEffectAbilityHandlerBase
{
	GENERATED_BODY()
	
public:
	UEndlessRustAbilityHandler();
	
	void ExecuteEffect(AActor* TargetActor, const FStatusEffect& StatusEffect) override;
};
