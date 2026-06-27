// Property of Paracosm.


#include "Weapons/Abilities/MeleeWeapon/BasicSlash.h"
#include "Character/TimberPlayableCharacter.h"


UBasicSlash::UBasicSlash()
{
	ABILITY_NAME = "BasicSlash";
	ValidationType = EAbilityValidation::NoResourceCost;
	WeaponAbilityType = EForWeaponType::MeleeWeapon;
	InputRequirement = EAbilityInputRequirement::Pressed;
	bNeedsProjectileData = false;
}

void UBasicSlash::Execute(FAbilityContext Context)
{
	BasicSlashContext = Context;
	ATimberPlayableCharacter* OwningCharacter = Cast<ATimberPlayableCharacter>(BasicSlashContext.Instigator);

	if (bComboInitiated || !BasicSlashContext.CombatComponent->bCanMeleeAttack)
	{
		return;
	}
	
	if (OwningCharacter && BasicSlashMontage)
	{
		//Anim Montage will reset this with Notifies
		//Context.CombatComponent->bCanMeleeAttack = true;

		//Combo window is open and available, and the combo hasn't been initiated yet.
		// Can NOT happen on the first Input Press - bComboWindowOpen is set to true from a Notify from the Ability Animation
		// Checking that !bComboIntiated is false to make sure that multiple presses in one window does increment the SectionIndex.
		if (bComboWindowOpen && !bComboInitiated )
		{
			
			//We dont call an animation to play here, it gets called in the Anim BP in a Notify.
			// Reasoning is we can mark with a notify exactly where we want the 2nd/3rd Sections to start playing.
			// We just set the vars here so the Notify event knows whether or not to play the next sections.
			SectionIndex++;
			UE_LOG(LogTemp, Warning, TEXT("Combo Window Open. Section Index: %i"), SectionIndex);
			
			//Tells melee weapon to initiate combo - Can Happen on the Subsequent Calls, after the first Input Press.
			bComboInitiated = true;

			BasicSlashContext.CombatComponent->bCanMeleeAttack = false;
			
			UE_LOG(LogTemp, Warning, TEXT("Combo Initiated"));
	
		}
		else if (SectionIndex == BasicSlashMontage->GetNumSections() - 1)
		{
			/*UE_LOG(LogTemp, Warning, TEXT("On last Phase of Combo."));*/
		}
		else if (!bComboWindowOpen && SectionIndex == 0)
		{
			BasicSlashContext.CombatComponent->bCanMeleeAttack = false;
			
			/*This is the first Swing of the Combo.*/
			UE_LOG(LogTemp, Warning, TEXT("Initial Combo Attack Play Animation."));
			FName SectionName = BasicSlashMontage->GetSectionName(SectionIndex);
			BasicSlashContext.CombatComponent->PlayCharacterAnimationMontage(BasicSlashMontage, SectionName, 1.0f, true);
		}
	}
}

void UBasicSlash::HandleMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
	//Is Not Fired when Jumping to Sections.
	Super::HandleMontageEnded(Montage, bInterrupted);
	
	UE_LOG(LogTemp, Warning, TEXT("Montage triggering HandleMontageEnded. %s"), *Montage->GetName());
	UE_LOG(LogTemp, Warning, TEXT("Section: %i"), SectionIndex);
	
	if (Montage == BasicSlashMontage && BasicSlashContext.CombatComponent )
	{
		
		if (bInterrupted)
		{
			UE_LOG(LogTemp, Warning, TEXT("Basic Slash Interrupted."));
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("Basic Slash Ended."));
			ResetComboData();
		}
	}
}

void UBasicSlash::ResetComboData()
{
	SectionIndex = 0;
	bComboInitiated = false;
	bComboWindowOpen = false;
	BasicSlashContext.CombatComponent->CurrentWeaponAbility = nullptr;
	BasicSlashContext.CombatComponent->bCanMeleeAttack = true;
}
