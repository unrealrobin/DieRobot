// Property of Paracosm.


#include "Synergy/EffectLogic/DerivedHandlers/ElectrocuteAbilityHandler.h"

#include "Character/Enemies/TimberEnemyCharacter.h"
#include "Components/StatusEffect/StatusEffectHandlerComponent.h"

UElectrocuteAbilityHandler::UElectrocuteAbilityHandler()
{
}

void UElectrocuteAbilityHandler::ExecuteEffect(AActor* TargetActor,
                                               const FStatusEffect& StatusEffect)
{
	Super::ExecuteEffect(TargetActor, StatusEffect);

	if (!StatusEffect.SecondaryEffect) return;
	
	TArray<AActor*> ActorsInRadius = GetActorsInRadius_IncludeTarget(TargetActor, 500.0f);
	TArray<AActor*> SelectedActors = SortActorsByDistance(ActorsInRadius, false, 3);
	
	for (AActor* ClosestNActor : SelectedActors)
	{
		//Apply an effect or Damage to these actors.
		if (!IsValid(ClosestNActor)) continue;
		
		ATimberEnemyCharacter* Enemy = Cast<ATimberEnemyCharacter>(ClosestNActor);
		if (IsValid(Enemy))
		{
			Enemy->StatusEffectHandler->AddStatusEffectToComponent(StatusEffect.SecondaryEffect->StatusEffect);
		}
	}
	
	UE_LOG(LogTemp, Warning, TEXT("Electrocute Ability Executed."));
}
