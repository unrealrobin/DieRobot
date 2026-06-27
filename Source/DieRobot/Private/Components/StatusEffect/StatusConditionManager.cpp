// Property of Paracosm.


#include "Components/StatusEffect/StatusConditionManager.h"

#include "NiagaraFunctionLibrary.h"
#include "Character/Enemies/TimberEnemyCharacter.h"
#include "Components/CapsuleComponent.h"
#include "Data/DataAssets/StatusEffects/StatusEffectDefinition.h"
#include "Subsystems/SynergySystem/SynergySystem.h"
#include "Types/EffectConditionTypes.h"


// Sets default values for this component's properties
UStatusConditionManager::UStatusConditionManager()
{
	PrimaryComponentTick.bCanEverTick = false;
}

// Called when the game starts
void UStatusConditionManager::BeginPlay()
{
	Super::BeginPlay();
	
	Owner = GetOwner();

	if (!IsValid(Owner))
	{
		UE_LOG(LogTemp, Warning, TEXT("Status Condition Manager: Owner is not set or invalid!"));
	}
}

FEffectConditionContext UStatusConditionManager::GenerateEffectConditionContext(AActor* TargetActor)
{
	FEffectConditionContext EffectConditionContext;

	EffectConditionContext.TargetActor = TargetActor;
	//Stores the Targets Gameplay Tag Container and Active Status Effects TArray.
	EffectConditionContext.TargetStatusEffectHandlerComponent = TargetActor->FindComponentByClass<UStatusEffectHandlerComponent>();
	EffectConditionContext.WorldContext = GetWorld();
	EffectConditionContext.SourceActor = Owner;
	
	return EffectConditionContext;
}



void UStatusConditionManager::ResolveEffect(TArray<UStatusEffectDefinition*> EffectDefinitionsArray, AActor* TargetActor)
{
	if (!IsValid(TargetActor)) return;

	USynergySystem* SynSub = GetWorld()->GetGameInstance()->GetSubsystem<USynergySystem>();
	if (!IsValid(SynSub)) return;
	
	FEffectConditionContext Context = GenerateEffectConditionContext(TargetActor);

	//Loops through each effect in this order: Ultimate -> Major -> Minor
	// If any Condition is met, the loop is stopped. Ensures only 1 Effect Per Resolve is called.
	for (int i = EffectDefinitionsArray.Num() - 1; i >= 0; i--)
	{
		UStatusEffectDefinition* EffectDefinition = EffectDefinitionsArray[i];

		bool bConditionMet = EffectDefinition->StatusEffectCondition->EvaluateCondition(Context);

		if (bConditionMet)
		{
			if (ATimberEnemyCharacter* Enemy = Cast<ATimberEnemyCharacter>(Context.TargetActor))
			{
				if (!IsValid(Enemy)) return;
				
				//Displays the Critical Synergy Effect Name in Floating Damage UI.
				if (EffectDefinition->StatusEffectAsset->StatusEffect.EffectLevel == EStatusEffectLevel::Ultimate)
				{
					FName TagName = SynSub->GetLastNameOfGameplayTag(EffectDefinition->StatusEffectAsset->StatusEffect.EffectIdTag);
					Enemy->SpawnEffectNameUI(TagName, EffectDefinition->StatusEffectAsset);
				}
				
				Enemy->StatusEffectHandler->AddStatusEffectToComponent(EffectDefinition->StatusEffectAsset->StatusEffect);

				//Play Trap Effect VFX
				if (EffectDefinition->StatusEffectAsset->StatusEffect.PrimaryNiagaraVFX)
				{
					PlayEffectVFX(EffectDefinition->StatusEffectAsset->StatusEffect.PrimaryNiagaraVFX, Enemy);
				}
				
				//Play Trap Effect SFX, should be played at Target Actor Location and should use Range Based Attenuation.
			}
			break;
		}
	}
}

//TODO:: THis plays all VFX at the Center of the Actor on the floor or at the Bottom of the Root Capsule Component. (Wont work for Each VFX type. Needs to be expanded.)
void UStatusConditionManager::PlayEffectVFX(TObjectPtr<UNiagaraSystem> VFX, AActor* TargetActor)
{
	if (VFX->IsValid() && GetWorld())
	{
		UCapsuleComponent* Capsule = Cast<UCapsuleComponent>(TargetActor->GetRootComponent());

		if (!Capsule) return;

		float CapsuleHalfHeight = Capsule->GetScaledCapsuleHalfHeight();

		//This is the middle location of the Capsule Component (Waist Area)
		FVector TargetLocation = TargetActor->GetActorLocation();

		//Offsetting Down the Z by the HalfHeight of the Capsule.
		FVector AdjustedLocation = FVector(TargetLocation.X, TargetLocation.Y, TargetLocation.Z - CapsuleHalfHeight);
		
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), VFX, AdjustedLocation, FRotator::ZeroRotator);
	}
}


