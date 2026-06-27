// Property of Paracosm.

#pragma once

#include "CoreMinimal.h"
#include "EffectConditionBase.h"
#include "EffectCondition_StackCount.generated.h"

/**
 * 
 */
UCLASS()
class DIEROBOT_API UEffectCondition_StackCount : public UEffectConditionBase
{
	GENERATED_BODY()

	UEffectCondition_StackCount();

public:

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tag")
	FGameplayTag Tag;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tag")
	int StackCount = 0;
	
	bool EvaluateCondition(const FEffectConditionContext& Context) const override;
};
