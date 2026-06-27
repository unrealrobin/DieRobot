// Property of Paracosm Industries.


#include "Character/DieRobotAnimInstance.h"
#include "Character/DieRobotPlayableCharacter.h"
#include "Controller/DieRobotPlayerController.h"

void UDieRobotAnimInstance::NativeBeginPlay()
{
	Super::NativeBeginPlay();
}

void UDieRobotAnimInstance::OnMontageEnded(UAnimMontage* Montage)
{
	//Handles Ending of Some Montage
}

void UDieRobotAnimInstance::UpdateOwnerWeaponState(EOwnerWeaponState OwnerWeaponState)
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

