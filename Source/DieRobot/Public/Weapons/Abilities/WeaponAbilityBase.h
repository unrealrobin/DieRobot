// Property of Paracosm.

#pragma once

#include "CoreMinimal.h"
#include "Components/Combat/CombatComponent.h"
#include "UObject/Object.h"
#include "WeaponAbilityBase.generated.h"

class UMetaSoundSource;
class UNiagaraSystem;

UENUM(BlueprintType)
enum class EAbilityInputRequirement: uint8
{
	Pressed UMETA(DisplayName = "Pressed"),
	HoldOnly UMETA(DisplayName = "HoldAndRelease"),
	Default UMETA(DisplayName = "Default"),
	
};

UENUM(BlueprintType)
enum class EAbilityValidation: uint8
{
	NoResourceCost UMETA(DisplayName = "NoResourceCost"),
	RequiresPower UMETA(DisplayName = "RequiresPower"),
	RequiresAmmo UMETA(DisplayName = "RequiresAmmo"),
	Default UMETA(DisplayName = "Default"),
};

UENUM(BlueprintType)
enum class EForWeaponType: uint8
{
	RangedWeapon UMETA(DisplayName = "RangedWeapon"),
	MeleeWeapon UMETA(DisplayName = "MeleeWeapon"),
	Default UMETA(DisplayName = "Default"),
};


/*struct FAbilityContext;*/
/**
 * 
 */
UCLASS(Abstract, Blueprintable)
class DIEROBOT_API UWeaponAbilityBase : public UObject
{
	GENERATED_BODY()

public:

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ability Details", meta=(DisplayName="Ability Name"))
	FName ABILITY_NAME = "NAME_NOT_SET";

	/*How the combat Controller should validate this ability*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ability Details")
	EAbilityValidation ValidationType = EAbilityValidation::Default;

	/*The type of weapon this ability is for.*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ability Details")
	EForWeaponType WeaponAbilityType = EForWeaponType::Default;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ability Details")
	EAbilityInputRequirement InputRequirement = EAbilityInputRequirement::Pressed;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ability Details")
	float PowerCost = 5.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ability Details")
	bool bNeedsProjectileData = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ability FX")
	UNiagaraSystem* NiagaraEffect = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ability FX")
	UMetaSoundSource* SoundFX = nullptr;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ability FX")
	FName SoundFXTrackName = "TRACK_NAME_NOT_SET";
	
	/*
	 * All Subclasses must Implement this function.
	 * This is what gets called by the Combat Component to initiate this Ability.
	 */
	virtual void Execute(FAbilityContext Context) PURE_VIRTUAL(UWeaponAbilityBase::Execute,);

	/*Used in Press&Release Abilities*/
	virtual void Execute_Completed(FAbilityContext Context){};
	virtual void Execute_Triggered(FAbilityContext Context){};
	virtual void Execute_Cancelled(FAbilityContext Context){};

	//Overridden in Child Ability Classes
	virtual void HandleMontageEnded(UAnimMontage* Montage, bool bInterrupted){};

	/*
	 * Clears the Stored Ability on the Combat Component
	 * Required for All Abilities.
	 */
	virtual void HandleCleanup(FAbilityContext Context);


protected:
	
	void PlayNiagaraEffectAtLocation(UNiagaraSystem* NiagaraSystem, FVector Location, FRotator Rotation = FRotator::ZeroRotator, bool AutoDestroy = true, bool AutoActive = true);
	void PlayNiagaraEffectAttached(UNiagaraSystem* NiagaraSystem, UPrimitiveComponent* Component, FName Name, const FVector& OffsetVector, const FRotator& Rotator, EAttachLocation::Type SnapToTargetIncludingScale, bool AutoDestroy = false, bool AutoActive = true);

	/* The Sound Effect MUST be in the SFX Library Data Asset */
	void PlayEffectSFX(FVector SoundLocation, FName TrackName);
};
