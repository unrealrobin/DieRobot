// Property of Paracosm.


#include "Data/DataAssets/StatusEffects/EffectConditions/EffectCondition_Always.h"

UEffectCondition_Always::UEffectCondition_Always()
{
	EffectConditionType = EEffectConditionType::Always;
}

bool UEffectCondition_Always::EvaluateCondition(const FEffectConditionContext& Context) const
{
	return true;
}
