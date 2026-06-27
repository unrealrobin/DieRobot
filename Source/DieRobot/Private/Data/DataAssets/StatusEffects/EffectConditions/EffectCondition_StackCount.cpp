// Property of Paracosm.


#include "Data/DataAssets/StatusEffects/EffectConditions/EffectCondition_StackCount.h"

UEffectCondition_StackCount::UEffectCondition_StackCount()
{
	EffectConditionType = EEffectConditionType::StackCount;
}

bool UEffectCondition_StackCount::EvaluateCondition(const FEffectConditionContext& Context) const
{
	
	
	if (Context.TargetStatusEffectHandlerComponent && Context.TargetStatusEffectHandlerComponent->StatusEffectIdTagContainer.Num() > 0)
	{
		//TODO:: 
		//Find the Tag in the Container, Get the Associated Status Effect, and Check the Stack Count on the Status Effect.
		// If > StackCount, return true.
		return true;
		
	}
	return false;
}
