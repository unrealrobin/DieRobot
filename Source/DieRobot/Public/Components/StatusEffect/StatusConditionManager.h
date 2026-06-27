// Property of Paracosm.

#pragma once

#include "CoreMinimal.h"
#include "NiagaraSystem.h"
#include "Components/ActorComponent.h"
#include "StatusConditionManager.generated.h"


class UStatusEffectDefinition;
struct FEffectConditionContext;
/*
 * This class is a component set on trap that evaluates a Traps Effect Conditions
 * and applies the condition to the Target Actor if the conditions are met.
 *
 * The Status Effect and Effect Condition are passed to this Component.
 *
 * The Status Effect is then passed to the Characters Status Effect Handler Component
 */

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class DIEROBOT_API UStatusConditionManager : public UActorComponent
{
	GENERATED_BODY()

public:
	UStatusConditionManager();

	void ResolveEffect(TArray<UStatusEffectDefinition*> EffectDefinitionsArray, AActor* TargetActor);
	
protected:
	
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Data")
	AActor* Owner = nullptr;

	FEffectConditionContext GenerateEffectConditionContext( AActor* TargetActor);

private:

	void PlayEffectVFX(TObjectPtr<UNiagaraSystem> VFX, AActor* TargetActor);
	//void PlayEffectSFX(TObjectPtr<UMetaSoundSource>, AActor* TargetActor);
	
};
