// Property of Paracosm.

#pragma once

#include "CoreMinimal.h"
#include "Synergy/EffectLogic/EffectAbilityHandlerBase.h"
#include "ElectrocuteAbilityHandler.generated.h"

/**
 * 
 */
UCLASS()
class DIEROBOT_API UElectrocuteAbilityHandler : public UEffectAbilityHandlerBase
{
	GENERATED_BODY()

public:

	UElectrocuteAbilityHandler();
	
	void ExecuteEffect(AActor* TargetActor, const FStatusEffect& StatusEffect) override;
};
