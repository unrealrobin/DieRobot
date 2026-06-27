// Property of Paracosm.


#include "Data/DataAssets/StatusEffects/EffectConditions/EffectCondition_Chance.h"

UEffectCondition_Chance::UEffectCondition_Chance()
{
	EffectConditionType = EEffectConditionType::Chance;
}

bool UEffectCondition_Chance::EvaluateCondition(const FEffectConditionContext& Context) const
{
	float Roll = FMath::FRandRange(0.0f, 1.0f);

	if (Roll <= Chance)
	{
		return true;
	}
	
	return false;
}
