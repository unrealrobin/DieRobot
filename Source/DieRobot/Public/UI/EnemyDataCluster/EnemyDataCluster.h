// Property of Paracosm.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "EnemyDataCluster.generated.h"

struct FGameplayTag;
class ATimberEnemyCharacter;
/**
 * 
 */
UCLASS()
class DIEROBOT_API UEnemyDataCluster : public UUserWidget
{
	GENERATED_BODY()

public:

	virtual void NativeConstruct() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Data")
	ATimberEnemyCharacter* EnemyOwner;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Widgets")
	UUserWidget* HealthBarWidget;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Widgets")
	UUserWidget* StatusEffectBarWidget;

	void AddEffectToStatusEffectBar(FGameplayTag EffectTag);
	void RemoveEffectFromStatusEffectBar(FGameplayTag EffectTag);
};
