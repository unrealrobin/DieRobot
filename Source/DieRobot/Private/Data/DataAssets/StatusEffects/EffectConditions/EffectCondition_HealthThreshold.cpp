// Property of Paracosm.


#include "Data/DataAssets/StatusEffects/EffectConditions/EffectCondition_HealthThreshold.h"

#include "Character/Enemies/TimberEnemyCharacter.h"

UEffectCondition_HealthThreshold::UEffectCondition_HealthThreshold()
{
	EffectConditionType = EEffectConditionType::HealthThreshold;
}

bool UEffectCondition_HealthThreshold::EvaluateCondition(const FEffectConditionContext& Context) const
{
	ATimberEnemyCharacter* Enemy = Cast<ATimberEnemyCharacter>(Context.TargetActor);
	if (Enemy)
	{
		if (bIsPercentage)
		{
			float HealthPercentage = Enemy->CurrentHealth / Enemy->MaxHealth;
			
			if (HealthPercentage <= HealthThreshold)
			{
				return true;
			}
		}

		if (Enemy->CurrentHealth <= HealthThreshold)
		{
			return true;
		}
	}
	
	return false;
}
