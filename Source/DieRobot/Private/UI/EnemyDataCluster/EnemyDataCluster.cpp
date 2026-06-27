// Property of Paracosm.


#include "UI/EnemyDataCluster/EnemyDataCluster.h"

#include "GameplayTagContainer.h"
#include "UI/StatusEffects/StatusEffectBar.h"

void UEnemyDataCluster::NativeConstruct()
{
	Super::NativeConstruct();
	
}

void UEnemyDataCluster::AddEffectToStatusEffectBar(FGameplayTag EffectTag)
{
	if (StatusEffectBarWidget)
	{
		if (UStatusEffectBar* StatusBarWidget = Cast<UStatusEffectBar>(StatusEffectBarWidget))
		{
			StatusBarWidget->AddEmergentTagToBar(EffectTag);
		}
		UE_LOG(LogTemp, Warning, TEXT("Added Effect: %s to Status Effect Bar"), *EffectTag.ToString());
	}
}

void UEnemyDataCluster::RemoveEffectFromStatusEffectBar(FGameplayTag EffectTag)
{
	if (StatusEffectBarWidget)
	{
		if (UStatusEffectBar* StatusBarWidget = Cast<UStatusEffectBar>(StatusEffectBarWidget))
		{
			StatusBarWidget->RemoveEffectFromBar(EffectTag);
		}
		//UE_LOG(LogTemp, Warning, TEXT("Added Effect: %s to Status Effect Bar"), *EffectTag.ToString()
	}
}
