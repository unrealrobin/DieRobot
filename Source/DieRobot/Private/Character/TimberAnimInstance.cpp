// Property of Paracosm Industries.


#include "Character/TimberAnimInstance.h"
#include "Character/TimberPlayableCharacter.h"
#include "Controller/TimberPlayerController.h"

void UTimberAnimInstance::NativeBeginPlay()
{
	Super::NativeBeginPlay();
}

void UTimberAnimInstance::OnMontageEnded(UAnimMontage* Montage)
{
	//Handles Ending of Some Montage
}

void UTimberAnimInstance::UpdateOwnerWeaponState(EOwnerWeaponState OwnerWeaponState)
{
	AnimCurrentWeaponState = OwnerWeaponState;
	
	if (AnimCurrentWeaponState == EOwnerWeaponState::RangedWeaponEquipped)
	{
		bIsRangedWeaponEquipped = true;
	}
	else
	{
		bIsRangedWeaponEquipped = false;
	}
}

