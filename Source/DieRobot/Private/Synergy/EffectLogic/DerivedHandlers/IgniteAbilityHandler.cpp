// Property of Paracosm.


#include "Synergy/EffectLogic/DerivedHandlers/IgniteAbilityHandler.h"

UIgniteAbilityHandler::UIgniteAbilityHandler()
{
}

void UIgniteAbilityHandler::ExecuteEffect(AActor* TargetActor,
                                          const FStatusEffect& StatusEffect)
{
	Super::ExecuteEffect(TargetActor, StatusEffect);

	UE_LOG(LogTemp, Warning, TEXT("Executing Ignite Ability Logic."));
}
