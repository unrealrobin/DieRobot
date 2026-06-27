// Property of Paracosm.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Types/EffectConditionTypes.h"
#include "EffectConditionBase.generated.h"


/**
 * 
 */
UCLASS(Abstract)
class DIEROBOT_API UEffectConditionBase : public UDataAsset
{
	GENERATED_BODY()

public:

	//The Type of Conditional check to perform.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effect Condition")
	EEffectConditionType EffectConditionType = EEffectConditionType::Default;

	//To be Overwritten by Child Classes.
	UFUNCTION()
	virtual bool EvaluateCondition(const FEffectConditionContext& Context) const PURE_VIRTUAL(UEffectConditionAsset::EvaluateCondition, return false;);

};
