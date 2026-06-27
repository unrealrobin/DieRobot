#pragma once
#include "Data/DataAssets/StatusEffects/StatusEffectBase.h"
#include "DamagePayload.generated.h"

USTRUCT(BlueprintType)
struct FDamagePayload
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage Payload")
	float DamageAmount = 0.f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage Payload")
	AActor* DamageInstigator = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage Payload")
	AActor* DamageOwner = nullptr;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage Payload")
	FStatusEffect StatusEffect;
};
