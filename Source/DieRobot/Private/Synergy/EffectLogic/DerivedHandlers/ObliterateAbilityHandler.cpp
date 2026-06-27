// Property of Paracosm.


#include "Synergy/EffectLogic/DerivedHandlers/ObliterateAbilityHandler.h"

UObliterateAbilityHandler::UObliterateAbilityHandler()
{
}

void UObliterateAbilityHandler::ExecuteEffect(AActor* TargetActor,
                                              const FStatusEffect& StatusEffect)
{
	Super::ExecuteEffect(TargetActor, StatusEffect);

	UE_LOG(LogTemp, Warning, TEXT("Obliterate Ability Executed."));
}
