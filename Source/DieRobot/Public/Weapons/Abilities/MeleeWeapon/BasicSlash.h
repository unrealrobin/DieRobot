// Property of Paracosm.

#pragma once

#include "CoreMinimal.h"
#include "Weapons/Abilities/WeaponAbilityBase.h"
#include "BasicSlash.generated.h"

/**
 * 
 */
UCLASS()
class DIEROBOT_API UBasicSlash : public UWeaponAbilityBase
{
	GENERATED_BODY()

public:
	
	UBasicSlash();

	virtual void Execute(FAbilityContext Context) override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Ability Data")
	FAbilityContext BasicSlashContext;

	/* Ability Montage*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ability Data")
	UAnimMontage* BasicSlashMontage = nullptr;

	/* Current Anim Montage Section for Combo Animation */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combo")
	int32 SectionIndex = 0;
	
	/*
	 * true if the player attacked again during an open combo window.
	 * Combo windows open and close via Anim Montage Notifies
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combo")
	bool bComboInitiated = false;

	/*The Window for when additional attack inputs will be allowed to make combo attacks.*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combo")
	bool bComboWindowOpen = false;
	
private:
	
	virtual void HandleMontageEnded(UAnimMontage* Montage, bool bInterrupted) override;
	
	UFUNCTION(BlueprintCallable, Category = "Combo")
	void ResetComboData();
};
