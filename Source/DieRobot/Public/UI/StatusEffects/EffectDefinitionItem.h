// Property of Paracosm.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Data/DataAssets/StatusEffects/StatusEffectBase.h"
#include "Data/DataAssets/StatusEffects/EffectConditions/EffectConditionBase.h"
#include "EffectDefinitionItem.generated.h"


class UVerticalBox;
class UHorizontalBox;
class UWrapBox;
enum class EEffectConditionType : uint8;
class USynergySystem;
class ATrapBase;
class UStatusEffectDefinition;
class UTextBlock;

/**
 * 
 */
UCLASS()
class DIEROBOT_API UEffectDefinitionItem : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;

	void ConfigureDataDisplay();
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "StatusEffects")
	UStatusEffectDefinition* EffectDefinition;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "StatusEffects")
	ATrapBase* TrapDefaultObject = nullptr;

	UPROPERTY()
	USynergySystem* SynSub = nullptr;

protected:
	/*Effect Level Data*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effect Data", meta = (BindWidget))
	UTextBlock* EffectLevelTextBlock;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effect Data", meta = (BindWidget))
	UTextBlock* EffectNameTextBlock;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effect Data", meta = (BindWidget))
	UTextBlock* ConditionValueTextBlock;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effect Data", meta = (BindWidget))
	UHorizontalBox* InitialDamageBox;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effect Data", meta = (BindWidget))
	UTextBlock* InitialDamageTextBlock;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effect Data", meta = (BindWidget))
	UTextBlock* DamagePerTickTextBlock;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effect Data", meta = (BindWidget))
	UTextBlock* DOTDurationTextBlock;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effect Data", meta = (BindWidget))
	UTextBlock* TickIntervalTextBlock;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effect Data", meta = (BindWidget))
	UHorizontalBox* ModifierListContainer;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effect Data", meta = (BindWidget))
	TArray<FName> ModifierList;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effect Data", meta = (BindWidget))
	UHorizontalBox* RemovalEffectTagList;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effect Data", meta = (BindWidget))
	TArray<FName> RemovesEffectTagList;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Effect Data")
	FText EffectLevelValue;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Effect Data")
	FText EffectName;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Effect Data")
	FText ConditionValue;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Effect Data")
	float DamagePerTickValue = 0.f;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Effect Data")
	float Duration = 0.f;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Effect Data")
	float TickInterval = 0.f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effect Data")
	FLinearColor EffectColor;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Effect Data")
	EStatusEffectLevel EffectLevelType;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Effect Data")
	EEffectConditionType ConditionType;

	//TODO:: Fix Later as I am Hard Coding Emergent Rule Value linking here.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effect Data", meta = (BindWidget)) 
	UVerticalBox* EmergentTagDisplay; 
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "EmergentTagRuleNames")
	FText ConditionEmergentTag;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "EmergentTagRuleNames")
	FText Rule1Value1;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "EmergentTagRuleNames")
	FText Rule1Value2;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "EmergentTagRuleNames")
	FText Rule2Value1;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "EmergentTagRuleNames")
	FText Rule2Value2;
	
private:
	void SetEffectLevelInfo(EStatusEffectLevel EffectLevel);
	void CheckForInitialDamage(FStatusEffect& StatusEffect);
	void CheckForModifiers(FStatusEffect& StatusEffect);
	void HandleEffectConditionType(UEffectConditionBase* EffectCondition);
	void GetEffectName(FStatusEffect& StatusEffect);
	void ProcessDOTInfo(FStatusEffect& StatusEffect);
	void GetTagsForEmergentCombos(UStatusEffectDefinition* Def);
};
