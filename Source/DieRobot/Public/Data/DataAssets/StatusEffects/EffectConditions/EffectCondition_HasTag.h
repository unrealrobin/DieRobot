// Property of Paracosm.

#pragma once

#include "CoreMinimal.h"
#include "EffectConditionBase.h"
#include "EffectCondition_HasTag.generated.h"

/**
 * 
 */
UCLASS()
class DIEROBOT_API UEffectCondition_HasTag : public UEffectConditionBase
{
	GENERATED_BODY()

	UEffectCondition_HasTag();
	
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tag")
	FGameplayTag Tag;
	
	bool EvaluateCondition(const FEffectConditionContext& Context) const override;
	
};
