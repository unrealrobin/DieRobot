// Property of Paracosm.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Components/ActorComponent.h"
#include "StatusEffectHandlerComponent.generated.h"


class ATimberEnemyCharacter;
struct FStatusEffect;
class UStatusEffectBase;

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class DIEROBOT_API UStatusEffectHandlerComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	UStatusEffectHandlerComponent();
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Status Effects")
	TArray<FStatusEffect> ActiveStatusEffects;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Effect Tags")
	FGameplayTagContainer StatusEffectIdTagContainer;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "StatusEffects")
	ATimberEnemyCharacter* OwningEnemyCharacter = nullptr;
	
	void AddStatusEffectToComponent(FStatusEffect& Effect);
	void AddEmergentTag(FGameplayTag Tag, float Duration);
	void RemoveEmergentTag(FGameplayTag Tag);
	
	FName GetLastNameOfTag(FGameplayTag Tag);
	
protected:

	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	
	void HandleEffectInitialDamage(FStatusEffect& Effect);
	void HandleModifierTags(FStatusEffect& StatusEffect);
	void HandleModifierDebuffIsSlowed(const FStatusEffect& Effect, float MaxWalkSpeedBaseMultiplier);
	void HandleModifierDebuffHasDot(FStatusEffect& StatusEffect, float DeltaTime);
	void HandleModifierDebuffIsStunned(FStatusEffect& StatusEffect);
	void HandleModifierRemovals(FStatusEffect& StatusEffect);
	void HandleIsStackableEffect(FStatusEffect* Effect);
	
	void RemoveMultipleEffectsFromComponent(TArray<FStatusEffect>& Effects);
	void ResetEffectDuration(FStatusEffect& Effect);
	void RemoveEffectFromComponent(const FStatusEffect& Effect);
	
	bool CheckIfTagAlreadyExists(FGameplayTag TagToCheck);
	void HandleStunEffectRemoval(const FStatusEffect& Effect);

	/* Update Status Effect Bar on Characters*/
	void AddEffectToStatusEffectBar(FGameplayTag EffectTag);
	void RemoveEffectFromStatusEffectBar(FGameplayTag EffectTag);

	
private:
	/*
	 * Why do we Stage For Removal?
	 * We don't want to modify the ActiveStatusEffects array while iterating through it every tick. So we loop every status effect,
	 * and the last part of logic in tick clears any staged status effects for destruction.
	 */
	UPROPERTY()
	TArray<FStatusEffect> StagedForRemoval;

	FStatusEffect* FindEffectByIdTag(const FGameplayTag& Tag);

	void ProcessTagForSynergy(const FGameplayTag& Tag, FStatusEffect& Effect);

};
