// Property of Paracosm.


#include "UI/StatusEffects/EffectDefinitionItem.h"

#include "Components/HorizontalBox.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Components/WrapBox.h"
#include "Data/DataAssets/StatusEffects/StatusEffectDefinition.h"
#include "Data/DataAssets/StatusEffects/EffectConditions/EffectCondition_Chance.h"
#include "Data/DataAssets/StatusEffects/EffectConditions/EffectCondition_HasTag.h"
#include "Data/DataAssets/StatusEffects/EffectConditions/EffectCondition_HealthThreshold.h"
#include "Data/DataAssets/StatusEffects/EffectConditions/EffectCondition_StackCount.h"
#include "Subsystems/SynergySystem/SynergySystem.h"

void UEffectDefinitionItem::NativeConstruct()
{
	Super::NativeConstruct();
}

void UEffectDefinitionItem::ConfigureDataDisplay()
{
	//This is the creation of a Single Effect Here. Can be Burn, Scorch, or Ignite. Must Accomodate for Each Scenario.
	SynSub = GetGameInstance()->GetSubsystem<USynergySystem>();

	if (EffectDefinition && SynSub)
	{
		FStatusEffect StatusEffectStruct = EffectDefinition->StatusEffectAsset->StatusEffect;
		EStatusEffectLevel EffectLevel = StatusEffectStruct.EffectLevel;
		EffectColor = StatusEffectStruct.GetEffectColor();
		SetEffectLevelInfo(EffectLevel);
		GetEffectName(StatusEffectStruct);
		HandleEffectConditionType(EffectDefinition->StatusEffectCondition);
		CheckForInitialDamage(StatusEffectStruct);
		CheckForModifiers(StatusEffectStruct);
		HandleEffectConditionType(EffectDefinition->StatusEffectCondition);

		if (EffectLevel == EStatusEffectLevel::Ultimate)
		{
			GetTagsForEmergentCombos(EffectDefinition);
		}
		else
		{
			EmergentTagDisplay->SetVisibility(ESlateVisibility::Collapsed);
		}
	}
	else
	{
		if (!SynSub)
		{
			UE_LOG(LogTemp, Warning, TEXT("EffectDefinition or SynergySystem is not valid."));
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("EffectDefinition is not valid."));
		}
	}
}

void UEffectDefinitionItem::SetEffectLevelInfo(EStatusEffectLevel EffectLevel)
{
	EffectLevelType = EffectLevel;
	switch (EffectLevel)
	{
	case EStatusEffectLevel::Minor:
		EffectLevelValue = FText::FromString("Minor");
		break;
	case EStatusEffectLevel::Major:
		EffectLevelValue = FText::FromString("Major");
		break;
	case EStatusEffectLevel::Ultimate:
		EffectLevelValue = FText::FromString("Ultimate");
		break;
	case EStatusEffectLevel::Default:
		EffectLevelValue = FText::FromString("DEV_FIX");
		break;
	default:
		EffectLevelValue = FText::FromString("DEV_FIX");
		break;
	}
}

void UEffectDefinitionItem::CheckForInitialDamage(FStatusEffect& StatusEffect)
{
	if (StatusEffect.InitialDamage > 0)
	{
		InitialDamageTextBlock->SetText(FText::AsNumber(StatusEffect.InitialDamage));
	}
	else
	{
		InitialDamageBox->SetVisibility(ESlateVisibility::Collapsed);
	}
}

void UEffectDefinitionItem::CheckForModifiers(FStatusEffect& StatusEffect)
{
	if (StatusEffect.ModifierTagContainer.Num() > 0)
	{
		for (FGameplayTag ModifierTag : StatusEffect.ModifierTagContainer)
		{
			if (SynSub->GetLastNameOfGameplayTag(ModifierTag) == FName("IsSlowed"))
			{
				ModifierList.Add(FName("Slow"));
				continue;
			}

			if (SynSub->GetLastNameOfGameplayTag(ModifierTag) == FName("IsStunned"))
			{
				ModifierList.Add(FName("Stun"));
				continue;
			}

			if (SynSub->GetLastNameOfGameplayTag(ModifierTag) == FName("HasDOT"))
			{
				ModifierList.Add(FName("DOT"));
				ProcessDOTInfo(StatusEffect);
				continue;
			}

			if (SynSub->GetLastNameOfGameplayTag(ModifierTag) == FName("IsTakesMoreDamage"))
			{
				ModifierList.Add(FName("+Damage"));
				continue;
			}

			if (SynSub->GetLastNameOfGameplayTag(ModifierTag) == FName("IsIncreasedSpeed"))
			{
				ModifierList.Add(FName("+Speed"));
				continue;	
			}

			if (SynSub->GetLastNameOfGameplayTag(ModifierTag) == FName("Removal"))
			{
				FGameplayTag RemovedEffectTag = StatusEffect.TagsToRemove.First();
				FName RemovedEffectName = SynSub->GetLastNameOfGameplayTag(RemovedEffectTag);
				RemovesEffectTagList.Add(RemovedEffectName);
			}
		}
	}
}

void UEffectDefinitionItem::HandleEffectConditionType(UEffectConditionBase* EffectCondition)
{
	ConditionType = EffectCondition->EffectConditionType;
	switch (ConditionType)
	{
	case EEffectConditionType::Always:
		ConditionValue = FText::FromString("Always Triggers");
		break;
	case EEffectConditionType::HasTag:
		if (UEffectCondition_HasTag* HasTagCondition = Cast<UEffectCondition_HasTag>(EffectCondition))
		{
			if (SynSub)
			{
				FName TagName = SynSub->GetLastNameOfGameplayTag(HasTagCondition->Tag);
				ConditionValue = FText::FromName(TagName);
			}
			else
			{
				ConditionValue = FText::FromString("Enemy Has Tag");
			}
		}
		break;
	case EEffectConditionType::StackCount:
		if (UEffectCondition_StackCount* StackCountCondition = Cast<UEffectCondition_StackCount>(EffectCondition))
		{
			ConditionValue = FText::AsNumber(StackCountCondition->StackCount);
		}
		break;
	case EEffectConditionType::HealthThreshold:
		if (UEffectCondition_HealthThreshold* HealthThresholdCondition = Cast<UEffectCondition_HealthThreshold>(EffectCondition))
		{
			ConditionValue = FText::AsNumber(HealthThresholdCondition->HealthThreshold);
		}
		break;
	case EEffectConditionType::Chance:
		if (UEffectCondition_Chance* ChanceCondition = Cast<UEffectCondition_Chance>(EffectCondition))
		{
			ConditionValue = FText::AsNumber(ChanceCondition->Chance * 100);
		}
		break;
	default:
		ConditionValueTextBlock->SetVisibility(ESlateVisibility::Collapsed);
	}
}

void UEffectDefinitionItem::GetEffectName(FStatusEffect& StatusEffect)
{
	if (SynSub)
	{
		FName Name = SynSub->GetLastNameOfGameplayTag(StatusEffect.EffectIdTag);
		EffectName = FText::FromName(Name);
	}
	else
	{
		EffectName = FText::FromString("EFFECT_NAME");
	}
}

void UEffectDefinitionItem::ProcessDOTInfo(FStatusEffect& StatusEffect)
{
	DamagePerTickValue = StatusEffect.DamagePerTick;
	Duration = StatusEffect.Duration;
	TickInterval = StatusEffect.TickInterval;

	if (DamagePerTickValue <= 0)
	{
		DamagePerTickTextBlock->SetVisibility(ESlateVisibility::Collapsed);
	}

	if (Duration <= 0)
	{
		DOTDurationTextBlock->SetVisibility(ESlateVisibility::Collapsed);
	}

	if (TickInterval <= 0){
		TickIntervalTextBlock->SetVisibility(ESlateVisibility::Collapsed);
	}
}

void UEffectDefinitionItem::GetTagsForEmergentCombos(UStatusEffectDefinition* Def)
{
	if (SynSub)
	{
		if (Def->StatusEffectCondition && Def->StatusEffectCondition->EffectConditionType == EEffectConditionType::HasTag)
		{
			ConditionEmergentTag = FText::FromName(SynSub->GetLastNameOfGameplayTag(Cast<UEffectCondition_HasTag>(Def->StatusEffectCondition)->Tag));
		}
		if (UEffectCondition_HasTag* HasTagCondition = Cast<UEffectCondition_HasTag>(Def->StatusEffectCondition))
		{
			
			if (TArray<FGameplayTagContainer>* ArrayOfGPTContainers = SynSub->EmergentEffectComboRequirements.Find(HasTagCondition->Tag))
			{
				if (ArrayOfGPTContainers->Num() == 2)
				{
					//Ew gross fix after finding a publisher. At least I am checking the values exist.
					TArray<FGameplayTagContainer> DerefArray = *ArrayOfGPTContainers;
					Rule1Value1 = FText::FromName(SynSub->GetLastNameOfGameplayTag(DerefArray[0].GetByIndex(0)));
					Rule1Value2 = FText::FromName(SynSub->GetLastNameOfGameplayTag(DerefArray[0].GetByIndex(1)));
					Rule2Value1 = FText::FromName(SynSub->GetLastNameOfGameplayTag(DerefArray[1].GetByIndex(0)));
					Rule2Value2 = FText::FromName(SynSub->GetLastNameOfGameplayTag(DerefArray[1].GetByIndex(1)));
				}
			}
		}
	}
}
