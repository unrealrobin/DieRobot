// Property of Paracosm.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "NiagaraSystem.h"
#include "Engine/DataAsset.h"
#include "StatusEffectBase.generated.h"

class UEffectAbilityHandlerBase;


UENUM(BlueprintType)
enum class EStatusEffectLevel : uint8
{
	Minor UMETA(DisplayName = "Minor"),
	Major UMETA(DisplayName = "Major"),
	Ultimate UMETA(DisplayName = "Ultimate"),
	Emergent UMETA(DisplayName = "Emergent"),
	Default UMETA(DisplayName = "Default")
};

USTRUCT(BlueprintType)
struct FStatusEffectFontSize
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effect Level")
	EStatusEffectLevel EffectLevel = EStatusEffectLevel::Default;

	int32 GetStatusEffectFontSize(const EStatusEffectLevel InEffectLevel) const
	{
		switch (InEffectLevel)
		{
			case EStatusEffectLevel::Minor:
				return 12;
			case EStatusEffectLevel::Major:
				return 16;
			case EStatusEffectLevel::Emergent:
				return 18;
			case EStatusEffectLevel::Ultimate:
				return 20;
			default:
				return 12;
		}
	}
};

UENUM(BlueprintType)
enum class EStatusEffectColorType : uint8
{
	Fire UMETA(DisplayName = "Fire"),
	Frost UMETA(DisplayName = "Frost"), 
	Corrosion UMETA(DisplayName = "Corrosion"),
	Arc UMETA(DisplayName = "Arc"),
	Physical UMETA(DisplayName = "Physical"),
	Default UMETA(DisplayName = "Default")
};

USTRUCT(BlueprintType)
struct FStatusEffectColors
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Effect Color Type")
	EStatusEffectColorType EffectColorType = EStatusEffectColorType::Default;

	/* Default Colors for Each Element Type */
	FLinearColor FireColor = FColor::FromHex(TEXT("FF7800")).ReinterpretAsLinear();     
	FLinearColor FrostColor = FColor::FromHex(TEXT("00DAFF")).ReinterpretAsLinear();      
	FLinearColor Arc = FColor::FromHex(TEXT("F6FF02")).ReinterpretAsLinear();       
	FLinearColor Corrosion = FColor::FromHex(TEXT("00FF1C")).ReinterpretAsLinear();   
	FLinearColor Physical = FColor::FromHex(TEXT("BB2928")).ReinterpretAsLinear();
	
	FLinearColor GetColorFromType(EStatusEffectColorType ColorType) const
	{
		switch (ColorType)
		{
			case EStatusEffectColorType::Fire:
				return FireColor;
			case EStatusEffectColorType::Frost:
				return FrostColor;
			case EStatusEffectColorType::Corrosion:
				return Corrosion;
			case EStatusEffectColorType::Arc:
				return Arc;
			case EStatusEffectColorType::Physical:
				return Physical;
			default:
				return FLinearColor::White;
		}
	}

};


USTRUCT(BlueprintType)
struct FStatusEffect
{
	GENERATED_BODY()

	// --- Required: Identity & Behavior ---
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effect Tags")
	FGameplayTag EffectIdTag;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Description")
	FText EffectDescription;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effect Tags")
	FGameplayTagContainer ModifierTagContainer;

	//Only gets checked if Modifier TagContainer contains a Removal Modifier.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effect Tags")
	FGameplayTagContainer TagsToRemove;

	//Duration of the effect before Automatic removal.
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Duration = 5.f; // How long the status lasts

	//Ticks every Tick Interval Seconds. e.g., 1.0f Ticks ever 1 Second.
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float TickInterval = 0.f; 

	//Damage Applied at every Tick Interval
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float DamagePerTick = 0.f; // Applied each tick

	//Any Initial Damage Caused by Effect.
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float InitialDamage = 0.f; // Dealt once on application

	//Always should be <= Max Stacks
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	int CurrentStacks = 1;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int MaxStacks = 100;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Secondary Effect")
	UStatusEffectBase* SecondaryEffect = nullptr;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "VFX")
	TObjectPtr<UNiagaraSystem> PrimaryNiagaraVFX;

	/* Color
	 *
	 * We Choose A Type in Editor for Each Status effect, depending on the Effects Element Type
	 * The Color can then be accessed through GetEffectColor()
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effect Colors")
	EStatusEffectColorType EffectColorType = EStatusEffectColorType::Default;
	
	FLinearColor GetEffectColor() const
	{
		static const FStatusEffectColors StatusEffectColors;
		return StatusEffectColors.GetColorFromType(EffectColorType);
	}

	/* Size
	 * This is defined similarly to the Color.
	 * A Status Effect Type will define the Size of the Text displayed on Screen.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effect Size")
	EStatusEffectLevel EffectLevel = EStatusEffectLevel::Default;
	
	int32 GetEffectTextSize() const
	{
		static const FStatusEffectFontSize StatusEffectFontSize;
		return StatusEffectFontSize.GetStatusEffectFontSize(EffectLevel);
	}

	/*Ability Effect Logic*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effect Ability")
	TSubclassOf<UEffectAbilityHandlerBase> EffectAbilityHandlerClass;
	
	/* Used for Runtime Effect Tracking - Not to Be Modified*/
	float TimeRemaining = 0.f;
	float TickAccumulator = 0.f;
};

/**
 * 
 */
UCLASS()
class DIEROBOT_API UStatusEffectBase : public UDataAsset
{
	GENERATED_BODY()

public:

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effect")
	FStatusEffect StatusEffect;
	
};
