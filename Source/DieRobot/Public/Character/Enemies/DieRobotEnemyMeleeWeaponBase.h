// Property of Paracosm Industries.

#pragma once

#include "CoreMinimal.h"
#include "DieRobotEnemyMeleeBase.h"
#include "DieRobotEnemyMeleeWeaponBase.generated.h"

class ADieRobotWeaponBase;

UCLASS()
class DIEROBOT_API ADieRobotEnemyMeleeWeaponBase : public ADieRobotEnemyMeleeBase
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	ADieRobotEnemyMeleeWeaponBase();

	/*UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy Info")
	float MeleeCharacterMovementSpeed = 400.f;*/

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon Info")
	TSubclassOf<ADieRobotWeaponBase> MeleeWeaponClassName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Weapon Info")
	ADieRobotWeaponBase* EquippedWeapon = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Animations)
	UAnimMontage* WeaponAttacksMontage = nullptr;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UFUNCTION()
	void EquipMeleeWeapon(TSubclassOf<ADieRobotWeaponBase> WeaponClassName);

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;
};
