// Property of Paracosm.

#pragma once

#include "CoreMinimal.h"
#include "EffectConditionBase.h"
#include "EffectCondition_HealthThreshold.generated.h"

/**
 * 
 */
UCLASS()
class DIEROBOT_API UEffectCondition_HealthThreshold : public UEffectConditionBase
{
	GENERATED_BODY()

	UEffectCondition_HealthThreshold();

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Health Threshold")
	float HealthThreshold = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Health Threshold")
	bool bIsPercentage = true;

	bool EvaluateCondition(const FEffectConditionContext& Context) const override;
};
