// Property of Paracosm.


#include "Components/StatusEffect/StatusEffectHandlerComponent.h"
#include "AI/TimberAiControllerBase.h"
#include "Character/Enemies/TimberEnemyCharacter.h"
#include "Components/CapsuleComponent.h"
#include "Components/WidgetComponent.h"
#include "Data/DataAssets/StatusEffects/StatusEffectBase.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Subsystems/SynergySystem/SynergySystem.h"
#include "Types/Combat/DamagePayload.h"
#include "UI/EnemyDataCluster/EnemyDataCluster.h"

UStatusEffectHandlerComponent::UStatusEffectHandlerComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UStatusEffectHandlerComponent::BeginPlay()
{
	Super::BeginPlay();

	OwningEnemyCharacter = Cast<ATimberEnemyCharacter>(GetOwner());
	if (!IsValid(OwningEnemyCharacter))
	{
		UE_LOG(LogTemp, Error, TEXT("StatusEffectHandlerComponent could not get Owning Enemy Character."));
	}
	
}

void UStatusEffectHandlerComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	//Going through Each Active Status Effect.
	for (FStatusEffect& StatusEffect : ActiveStatusEffects)
	{
		/*If the effect still has time remaining*/
		if (StatusEffect.TimeRemaining > 0.f)
		{
			StatusEffect.TimeRemaining -= DeltaTime;
			
			HandleModifierDebuffHasDot(StatusEffect, DeltaTime);
		}
		else
		{
			StagedForRemoval.Add(StatusEffect);
		}
	}
	if (StagedForRemoval.Num() > 0)
	{
		RemoveMultipleEffectsFromComponent(StagedForRemoval);
	}
}

void UStatusEffectHandlerComponent::AddStatusEffectToComponent(FStatusEffect& Effect)
{
	if (!IsValid(OwningEnemyCharacter)) return;
	
	bool bTagExists = CheckIfTagAlreadyExists(Effect.EffectIdTag);

	if (!bTagExists)
	{
		StatusEffectIdTagContainer.AddTag(Effect.EffectIdTag);
		
		ActiveStatusEffects.Add(Effect);

		//Initializing TimeRemaining to Start at Effect Duration - We subtract from Time Remaining Every Tick.
		Effect.TimeRemaining = Effect.Duration;
		
		HandleEffectInitialDamage(Effect);

		HandleModifierTags(Effect);
		
		//This adds the Effect Icon to the Status Effect Bar on the Enemy Character.
		//AddEffectToStatusEffectBar(Effect.EffectIdTag);
	}
	else
	{
		//If the ID Already existing in our array, we need to get the existing version of the effect. And add a stack. Then we need to reset the duration
		//because it's basically a new iteration of the effect at a higher stack.
		
		FStatusEffect* ExisitingStatusEffect = FindEffectByIdTag(Effect.EffectIdTag);
		if (ExisitingStatusEffect->MaxStacks > 1)
		{
			HandleIsStackableEffect(ExisitingStatusEffect);
		}
	}

	//Only needs to be done if the Status Effect is a Major Effect.
	ProcessTagForSynergy(Effect.EffectIdTag, Effect);
}

void UStatusEffectHandlerComponent::AddEmergentTag(FGameplayTag Tag, float Duration)
{
	/* Emergent Tags do not use Regular Channels for Addition. They don't use AddStatusEffectToComponent(), Avoiding ProcessTagForSynergy.*/
	if (!IsValid(OwningEnemyCharacter)) return;
	UE_LOG(LogTemp, Warning, TEXT("Emergent Tag: %s"), *Tag.ToString())
	
	AddEffectToStatusEffectBar(Tag);
	StatusEffectIdTagContainer.AddTag(Tag);
	/*
	 * We still do not Add an Emergent Status Effect to the Array of Status Effects. The Actual effects
	 * dont do anything, they simply store information about the type of effect. Used for UI Information.
	 */
	//LoadStatusEffect from Tag.
	USynergySystem* SynergySub = GetWorld()->GetGameInstance()->GetSubsystem<USynergySystem>();
	if (SynergySub)
	{
		//TODO:: Double look up, fix later - Map doesn't change.
		if (SynergySub->EmergentEffectsMap.Find(Tag))
		{
			//No Dereferencing a null ptr.
			UStatusEffectBase* EmergentStatusEffect = *SynergySub->EmergentEffectsMap.Find(Tag);
			if (IsValid(EmergentStatusEffect))
			{
				FName TagName = GetLastNameOfTag(Tag);
				OwningEnemyCharacter->SpawnEffectNameUI(TagName, EmergentStatusEffect);
			}
		}
	}

	FTimerHandle EmergentTimerHandle;
	TWeakObjectPtr<UStatusEffectHandlerComponent> WeakThis(this);
	if (GetWorld())
	{
		GetWorld()->GetTimerManager().SetTimer(EmergentTimerHandle, [WeakThis, Tag]()
		{
			if (!WeakThis.IsValid()) return;
			
			if (WeakThis->StatusEffectIdTagContainer.HasTagExact(Tag))
			{
				WeakThis->StatusEffectIdTagContainer.RemoveTag(Tag);
			}
			
		}, Duration, false);
	}
}

void UStatusEffectHandlerComponent::RemoveEmergentTag(FGameplayTag Tag)
{
	StatusEffectIdTagContainer.RemoveTag(Tag);

	//Removes the Visual Representation from the Data Cluster on Enemies.
	RemoveEffectFromStatusEffectBar(Tag);
}

void UStatusEffectHandlerComponent::ProcessTagForSynergy(const FGameplayTag& Tag, FStatusEffect& Effect)
{
	//Call to Synergy Subsystem to check for Synergies.
	//Synergy subsystem handles the application of Tags - Effect Logic for Lvl 3 Synergies.
	USynergySystem* SynSub = GetWorld()->GetGameInstance()->GetSubsystem<USynergySystem>();
	if (SynSub)
	{
		SynSub->ProcessTagForSynergy(Tag, this, Effect);
	}
}

void UStatusEffectHandlerComponent::HandleModifierTags(FStatusEffect& StatusEffect)
{
	if (!IsValid(OwningEnemyCharacter)) return;
	
	for (FGameplayTag ModifierTag : StatusEffect.ModifierTagContainer)
	{
		//Removal
		if (ModifierTag.MatchesTagExact(FGameplayTag::RequestGameplayTag("SynergySystem.Modifier.Removal")))
		{
			HandleModifierRemovals(StatusEffect);
			continue;
		}
		
		//Slow
		if (ModifierTag.MatchesTagExact(FGameplayTag::RequestGameplayTag("SynergySystem.Modifier.Debuff.IsSlowed")))
		{
			//TODO:: Update to use some slow amount from the FStatusEffect Struct.
			HandleModifierDebuffIsSlowed(StatusEffect, 0.5f);
			continue;
		}
		
		//Dot
		if (ModifierTag.MatchesTagExact(FGameplayTag::RequestGameplayTag("SynergySystem.Modifier.Debuff.HasDOT")))
		{
			continue;
		}
		//Stacks

		//Stunned (Freeze)
		if (ModifierTag.MatchesTagExact(FGameplayTag::RequestGameplayTag("SynergySystem.Modifier.Debuff.IsStunned")))
		{
			HandleModifierDebuffIsStunned(StatusEffect);
			continue;
		}
	}
}

void UStatusEffectHandlerComponent::HandleModifierDebuffHasDot(FStatusEffect& StatusEffect, float DeltaTime)
{
	if (!IsValid(OwningEnemyCharacter)) return;
	//Handle Any DOT Tags
	for (FGameplayTag Tag : StatusEffect.ModifierTagContainer)
	{
		if (Tag == FGameplayTag::RequestGameplayTag("SynergySystem.Modifier.Debuff.HasDOT"))
		{
			StatusEffect.TickAccumulator += DeltaTime;
			if (StatusEffect.TickAccumulator >= StatusEffect.TickInterval)
			{
				//Stacks multiply the Damage per Tick.
				float TotalDamage = StatusEffect.DamagePerTick * StatusEffect.CurrentStacks;

				FDamagePayload Payload;
				Payload.DamageAmount = TotalDamage;
				Payload.StatusEffect = StatusEffect;
				
				OwningEnemyCharacter->TakeDamage(Payload);
				
				//UE_LOG(LogTemp, Warning, TEXT ("Applied Total Damage Dot: %f"), TotalDamage);
				//Resetting the Tick Accumulator
				StatusEffect.TickAccumulator = 0;
			}
		}
	}
}

void UStatusEffectHandlerComponent::HandleModifierDebuffIsStunned(FStatusEffect& StatusEffect)
{
	if (!IsValid(OwningEnemyCharacter))
	{
		return;
	}

	UCapsuleComponent* CapsuleComp = OwningEnemyCharacter->GetCapsuleComponent();
	USkeletalMeshComponent* MeshComp = OwningEnemyCharacter->GetMesh();
	UCharacterMovementComponent* MoveComp = OwningEnemyCharacter->GetCharacterMovement();

	if (IsValid(CapsuleComp))
	{
		CapsuleComp->SetCollisionProfileName("DR_Stunned_NonBlocking");
	}
	if (IsValid(MeshComp))
	{
		MeshComp->GlobalAnimRateScale = 0.0f;
	}
	if (IsValid(MoveComp))
	{
		MoveComp->StopMovementImmediately();
	}

	ATimberAiControllerBase* AiController = Cast<ATimberAiControllerBase>(OwningEnemyCharacter->GetController());
	if (IsValid(AiController) && IsValid(AiController->BrainComponent))
	{
		AiController->BrainComponent->StopLogic("Is Stunned");
	}
}

void UStatusEffectHandlerComponent::HandleModifierDebuffIsSlowed(const FStatusEffect& Effect, float MaxWalkSpeedBaseMultiplier)
{
	/*
	 * When adding a Slow tag, this modifies the Speed to .5x
	 * When removing it reverts back to 1.0x
	 *
	 * Potential issue here if more than 1 effect can apply a slow, or if there are stacks, this totally removes any slow effects.
	 * Potential Solution is we have Slow States, and they evaluate every tick. Expensive.
	 */
	for (FGameplayTag Tag : Effect.ModifierTagContainer)
	{
		if (Tag.MatchesTagExact(FGameplayTag::RequestGameplayTag("SynergySystem.Modifier.Debuff.IsSlowed")))
		{
			if (IsValid(OwningEnemyCharacter) && IsValid(OwningEnemyCharacter->GetCharacterMovement()))
			{
				OwningEnemyCharacter->GetCharacterMovement()->MaxWalkSpeed = OwningEnemyCharacter->MaxWalkSpeedBase *  MaxWalkSpeedBaseMultiplier; 
				break;
			}
		}
	}
}

void UStatusEffectHandlerComponent::HandleModifierRemovals(FStatusEffect& StatusEffect)
{
	if (IsValid(OwningEnemyCharacter))
	{
		for (FGameplayTag TagToRemove : StatusEffect.TagsToRemove)
		{
			//Search for Tag in StatusEffect Array.
			for (FStatusEffect& ActiveEffect : ActiveStatusEffects)
			{
				if (ActiveEffect.EffectIdTag.MatchesTagExact(TagToRemove))
				{
					StagedForRemoval.Add(ActiveEffect);
				}
			}
		}
	}
	
}

void UStatusEffectHandlerComponent::HandleStunEffectRemoval(const FStatusEffect& Effect)
{
	/* REMOVE STUN */
	//Removing an effect that has IsStunned Tag.
	//The Issue here is what if multiple tags apply stunned we are removing that effect completely.
	if (Effect.ModifierTagContainer.HasTagExact(FGameplayTag::RequestGameplayTag("SynergySystem.Modifier.Debuff.IsStunned")))
	{
		if (IsValid(OwningEnemyCharacter))
		{
			if (OwningEnemyCharacter->GetCapsuleComponent())
			{
				OwningEnemyCharacter->GetCapsuleComponent()->SetCollisionProfileName("DR_EnemyCharacterCapsule");
			}
			if (OwningEnemyCharacter->GetMesh())
			{
				OwningEnemyCharacter->GetMesh()->GlobalAnimRateScale = 1.0f;
			}
		}
		ATimberAiControllerBase* AiController = Cast<ATimberAiControllerBase>(OwningEnemyCharacter->GetController());
		//Freezes Enemy logic
		if (IsValid(AiController) && IsValid(AiController->BrainComponent))
		{
			AiController->BrainComponent->RestartLogic();
		}
	}
}

void UStatusEffectHandlerComponent::HandleIsStackableEffect(FStatusEffect* Effect)
{
	if (Effect && Effect->CurrentStacks < Effect->MaxStacks)
	{
		//Increment the stacks if possible.
		Effect->CurrentStacks++;
	}

	//We want to reset the duration of the effect even if its at Max Stacks.
	ResetEffectDuration(*Effect);
}

void UStatusEffectHandlerComponent::HandleEffectInitialDamage(FStatusEffect& Effect)
{
	if (Effect.InitialDamage > 0.f)
	{
		FDamagePayload Payload;
		//Superfluous
		Payload.DamageAmount = Effect.InitialDamage;
		Payload.StatusEffect = Effect;
		if (IsValid(OwningEnemyCharacter))
		{
			OwningEnemyCharacter->TakeDamage(Payload);
		}
	}
}

void UStatusEffectHandlerComponent::AddEffectToStatusEffectBar(FGameplayTag EffectTag)
{
	//TODO::This only works with the Enemy Characters At the moment. Interface? 
	ATimberEnemyCharacter* EnemyCharacter = Cast<ATimberEnemyCharacter>(GetOwner());
	if (IsValid(EnemyCharacter))
	{
		if (IsValid(EnemyCharacter->DataClusterWidgetComponent))
		{
			UUserWidget* Widget = EnemyCharacter->DataClusterWidgetComponent->GetWidget();
			if (IsValid(Widget))
			{
				UEnemyDataCluster* DC = Cast<UEnemyDataCluster>(Widget);
				if (IsValid(DC) && IsValid(DC->StatusEffectBarWidget))
				{
					DC->AddEffectToStatusEffectBar(EffectTag);
					UE_LOG(LogTemp, Warning, TEXT("Added Effect to Status Effect Bar: %s"), *EffectTag.ToString())
				}
				else
				{
					UE_LOG(LogTemp, Warning, TEXT("Status Effect Bar Widget is Null"));
				}
			}
		}
	}
}

void UStatusEffectHandlerComponent::RemoveEffectFromStatusEffectBar(FGameplayTag EffectTag)
{
	ATimberEnemyCharacter* EnemyCharacter = Cast<ATimberEnemyCharacter>(GetOwner());
	if (IsValid(EnemyCharacter))
	{
		if (IsValid(EnemyCharacter->DataClusterWidgetComponent))
		{
			UUserWidget* Widget = EnemyCharacter->DataClusterWidgetComponent->GetWidget();
			if (IsValid(Widget))
			{
				UEnemyDataCluster* DC = Cast<UEnemyDataCluster>(Widget);
				if (IsValid(DC) && IsValid(DC->StatusEffectBarWidget))
				{
					DC->RemoveEffectFromStatusEffectBar(EffectTag);
				}
			}
		}
	}
}

bool UStatusEffectHandlerComponent::CheckIfTagAlreadyExists(FGameplayTag TagToCheck)
{
	 return StatusEffectIdTagContainer.HasTagExact(TagToCheck);
}

void UStatusEffectHandlerComponent::RemoveEffectFromComponent(const FStatusEffect& Effect)
{
	/*
	 * Why do we Stage For Removal?
	 * We don't want to modify the ActiveStatusEffects array while iterating through it every tick. So we loop every status effect,
	 * and the last part of logic in tick clears any staged status effects for destruction.
	 */
	HandleModifierDebuffIsSlowed(Effect, 1.0f); //Back to normal speed

	HandleStunEffectRemoval(Effect);
	
	//This would remove all status effects, even if there are multiple stacks/instances of the same effect.(Imagine 2 Slow traps in the same location.)
	ActiveStatusEffects.RemoveAll([Effect](const FStatusEffect& ActiveEffect)
	{
		return ActiveEffect.EffectIdTag == Effect.EffectIdTag;
	});

	//Status Effect Bar UI Icon (Above Enemy Heads)
	RemoveEffectFromStatusEffectBar(Effect.EffectIdTag);
	
	//Remove The Unique Id Tag from the Tag Container.
	StatusEffectIdTagContainer.RemoveTag(Effect.EffectIdTag);
}

void UStatusEffectHandlerComponent::RemoveMultipleEffectsFromComponent(TArray<FStatusEffect>& Effects)
{
	for (FStatusEffect& StatusEffect : Effects)
	{
		RemoveEffectFromComponent(StatusEffect);
	}

	StagedForRemoval.Empty();
}

void UStatusEffectHandlerComponent::ResetEffectDuration(FStatusEffect& Effect)
{
	//Resetting the Duration of the effect. Typically used on the application of a new stack.
	Effect.TimeRemaining = Effect.Duration;
}

FStatusEffect* UStatusEffectHandlerComponent::FindEffectByIdTag(const FGameplayTag& Tag)
{
	return ActiveStatusEffects.FindByPredicate([&Tag]( FStatusEffect& Effect)
	{
		return Effect.EffectIdTag == Tag;
	});
}

FName UStatusEffectHandlerComponent::GetLastNameOfTag(FGameplayTag Tag)
{
	FString TagName = Tag.GetTagName().ToString();
	TArray<FString> Parts;
	TagName.ParseIntoArray(Parts, TEXT("."), true);

	//Converts the String to Uppercase
	FName EffectName = Parts.Num() > 0 ? FName(*Parts.Last().ToUpper()) : NAME_None;

	return EffectName;
}



