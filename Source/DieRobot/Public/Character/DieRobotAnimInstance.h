// Property of Paracosm Industries.

#pragma once

#include "CoreMinimal.h"
#include "InputActionValue.h"
#include "Animation/AnimInstance.h"
#include "Components/Combat/CombatComponent.h"
#include "DieRobotAnimInstance.generated.h"


enum class EOwnerWeaponState : uint8;
enum class ECharacterState : uint8;
class ADieRobotPlayableCharacter;
class ADieRobotPlayerController;

/**
 * 
 */
UCLASS()
class DIEROBOT_API UDieRobotAnimInstance : public UAnimInstance
{
	GENERATED_BODY()

public:
	virtual void NativeBeginPlay() override;

	UFUNCTION()
	void OnMontageEnded(UAnimMontage* Montage);

	
	UPROPERTY(BlueprintReadOnly)
	FInputActionValue InputActionValue;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Character Info")
	ADieRobotPlayableCharacter* OwningPawn;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Character Info")
	ADieRobotPlayerController* PlayerController;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Character Info")
	bool CharacterNControllerInitialized = false;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Info")
	bool bIsReloading = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Info")
	bool bIsRangedWeaponEquipped = false;

	void UpdateOwnerWeaponState(EOwnerWeaponState OwnerWeaponState);

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Character Info")
	EOwnerWeaponState AnimCurrentWeaponState = EOwnerWeaponState::Unequipped;
};
