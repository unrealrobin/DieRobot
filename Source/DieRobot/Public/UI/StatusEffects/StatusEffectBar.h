// Property of Paracosm.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Blueprint/UserWidget.h"
#include "StatusEffectBar.generated.h"

class UHorizontalBox;
class UStatusEffectBarIcon;

/**
 * 
 */
UCLASS()
class DIEROBOT_API UStatusEffectBar : public UUserWidget
{
	GENERATED_BODY()

public:

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "StatusEffects")
	TSubclassOf<UUserWidget> StatusEffectBarIconClass;

	UPROPERTY(meta = (BindWidget))
	UHorizontalBox* StatusEffectIconContainer;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "StatusEffects")
	FGameplayTagContainer ActiveWidgetTags;

	void NativeConstruct() override;
	
	//Takes a GameplayTag and adds a Specific Icon for that Effect to the Status Effect Bar.
	UFUNCTION()
	void AddEmergentTagToBar(FGameplayTag EffectTag);

	UFUNCTION()
	void RemoveEffectFromBar(FGameplayTag EffectTag);

};
