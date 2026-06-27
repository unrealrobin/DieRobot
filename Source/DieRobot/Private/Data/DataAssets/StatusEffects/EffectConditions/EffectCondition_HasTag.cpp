// Property of Paracosm.


#include "Data/DataAssets/StatusEffects/EffectConditions/EffectCondition_HasTag.h"

UEffectCondition_HasTag::UEffectCondition_HasTag()
{

	EffectConditionType = EEffectConditionType::HasTag;
}

bool UEffectCondition_HasTag::EvaluateCondition(const FEffectConditionContext& Context) const
{

	//Check Context.TargetStatusEffectHandlerComponent TagContainer for Tag
	//FGameplayTagContainer StatusEffectIdTagContainer; < --- this is where the tags live

	if (Context.TargetStatusEffectHandlerComponent && Context.TargetStatusEffectHandlerComponent->StatusEffectIdTagContainer.Num() > 0)
	{
		if (Context.TargetStatusEffectHandlerComponent->StatusEffectIdTagContainer.HasTag(Tag))
		{
			return true;
		}
	}
	
	return false;
}
