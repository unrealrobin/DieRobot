// Property of Paracosm Industries.

#pragma once

#include "CoreMinimal.h"
#include "DieRobotEnemyCharacter.h"
#include "DieRobotEnemyRangedBase.generated.h"

class ADieRobotWeaponRangedBase;
class ADieRobotWeaponBase;

UCLASS()
class DIEROBOT_API ADieRobotEnemyRangedBase : public ADieRobotEnemyCharacter
{
	GENERATED_BODY()

public:
	ADieRobotEnemyRangedBase();
	
	virtual void Tick(float DeltaTime) override;

protected:
	virtual void BeginPlay() override;

	UFUNCTION()
	void EquipRangedWeapon();

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon Info")
	TSubclassOf<ADieRobotWeaponBase> RangedWeaponClassName = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon Info")
	ADieRobotWeaponRangedBase* EquippedWeapon = nullptr;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon Info")
	USceneComponent* AimStartPointComponent;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon Info")
	FName RangedSocketEquippedName = "EquippedRangeSocket";

	//Assists in Aim Offset Calculations in Animation Blueprint
	UFUNCTION(BlueprintCallable)
	void GetRotationToCurrentTarget();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="RangedTargetRotation")
	float PitchToTarget;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="RangedTargetRotation")
	float YawToTarget;
};
