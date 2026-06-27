// Property of Paracosm.

#pragma once

#include "CoreMinimal.h"
#include "EffectConditionBase.h"
#include "EffectCondition_Chance.generated.h"

/**
 * 
 */
UCLASS()
class DIEROBOT_API UEffectCondition_Chance : public UEffectConditionBase
{
	GENERATED_BODY()
	UEffectCondition_Chance();

public:

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chance")
	float Chance = 0.1f;

	bool EvaluateCondition(const FEffectConditionContext& Context) const override;
};
