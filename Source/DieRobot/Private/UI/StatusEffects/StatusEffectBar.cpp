// Property of Paracosm.


#include "UI/StatusEffects/StatusEffectBar.h"
#include "GameplayTagContainer.h"
#include "Components/HorizontalBox.h"
#include "UI/StatusEffects/StatusEffectBarIcon.h"


void UStatusEffectBar::NativeConstruct()
{
	Super::NativeConstruct();
}

void UStatusEffectBar::AddEmergentTagToBar(FGameplayTag EffectTag)
{
	
	if (!ActiveWidgetTags.HasTagExact(EffectTag))
	{
		UUserWidget* NewIcon = CreateWidget<UUserWidget>(GetWorld(), StatusEffectBarIconClass);
		if (UStatusEffectBarIcon* IconWidget = Cast<UStatusEffectBarIcon>(NewIcon))
		{
			//Checking the ID of the Effect Tag to determine the tint color for the icon.
			if (EffectTag == FGameplayTag::RequestGameplayTag("SynergySystem.EmergentEffect.Charred"))
			{
				IconWidget->StatusBarIconTint = FLinearColor::Red; 
			}
			else if (EffectTag == FGameplayTag::RequestGameplayTag("SynergySystem.EmergentEffect.Corroded"))
			{
				IconWidget->StatusBarIconTint = FLinearColor::Green; 
			}
			else if (EffectTag == FGameplayTag::RequestGameplayTag("SynergySystem.EmergentEffect.Wet"))
			{
				IconWidget->StatusBarIconTint = FLinearColor::Blue; 
			}
			else if (EffectTag == FGameplayTag::RequestGameplayTag("SynergySystem.EmergentEffect.Demolish"))
			{
				IconWidget->StatusBarIconTint = FLinearColor::Gray; 
			}
			else if (EffectTag == FGameplayTag::RequestGameplayTag("SynergySystem.EmergentEffect.Sparked"))
			{
				IconWidget->StatusBarIconTint = FLinearColor::Yellow; 
			}
			else
			{
				//For Status Effect attacks that shouldn't have an icon. These are Initial Damage only Effects.
				return;
			}

			//Setting the GameplayTag on the Widget - Used for Removal
			IconWidget->AssociatedEffectTag = EffectTag;
			ActiveWidgetTags.AddTag(EffectTag);
			StatusEffectIconContainer->AddChild(IconWidget);
		}
	}
}

void UStatusEffectBar::RemoveEffectFromBar(FGameplayTag EffectTag)
{
	if (IsValid(StatusEffectIconContainer))
	{
		TArray<UWidget*> IconWidgetsArray = StatusEffectIconContainer->GetAllChildren();
		for (UWidget* Widget : IconWidgetsArray)
		{
			UStatusEffectBarIcon* IconWidget = Cast<UStatusEffectBarIcon>(Widget);
			if (IconWidget)
			{
				//Comparing the Linked Tag to the StatusBarIcon to the Effect Tag We Want to Remove.
				if (IconWidget->AssociatedEffectTag == EffectTag)
				{
					StatusEffectIconContainer->RemoveChild(IconWidget);
					if (ActiveWidgetTags.HasTagExact(EffectTag))
					{
						ActiveWidgetTags.RemoveTag(EffectTag);
					}
					break;
				}
			}
		}

		//UE_LOG(LogTemp, Warning, TEXT("Removed Effect: %s from Status Effect Bar"), *EffectTag.ToString());
	}
}
