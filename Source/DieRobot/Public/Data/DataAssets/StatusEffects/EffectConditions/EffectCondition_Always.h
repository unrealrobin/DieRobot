// Property of Paracosm.

#pragma once

#include "CoreMinimal.h"
#include "EffectConditionBase.h"
#include "EffectCondition_Always.generated.h"

/**
 * 
 */
UCLASS()
class DIEROBOT_API UEffectCondition_Always : public UEffectConditionBase
{
	GENERATED_BODY()

	UEffectCondition_Always();

public:
	virtual bool EvaluateCondition(const FEffectConditionContext& Context) const override;
};
