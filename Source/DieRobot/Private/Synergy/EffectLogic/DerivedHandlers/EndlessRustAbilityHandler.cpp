// Property of Paracosm.


#include "Synergy/EffectLogic/DerivedHandlers/EndlessRustAbilityHandler.h"

UEndlessRustAbilityHandler::UEndlessRustAbilityHandler()
{
}

void UEndlessRustAbilityHandler::ExecuteEffect(AActor* TargetActor,
                                               const FStatusEffect& StatusEffect)
{
	Super::ExecuteEffect(TargetActor, StatusEffect);

	UE_LOG(LogTemp, Warning, TEXT("Endless Rust Ability Executed."));
}
