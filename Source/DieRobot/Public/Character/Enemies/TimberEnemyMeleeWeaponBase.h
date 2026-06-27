// Property of Paracosm Industries.

#pragma once

#include "CoreMinimal.h"
#include "TimberEnemyMeleeBase.h"
#include "TimberEnemyMeleeWeaponBase.generated.h"

class ATimberWeaponBase;

UCLASS()
class DIEROBOT_API ATimberEnemyMeleeWeaponBase : public ATimberEnemyMeleeBase
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	ATimberEnemyMeleeWeaponBase();

	/*UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy Info")
	float MeleeCharacterMovementSpeed = 400.f;*/

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon Info")
	TSubclassOf<ATimberWeaponBase> MeleeWeaponClassName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Weapon Info")
	ATimberWeaponBase* EquippedWeapon = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Animations)
	UAnimMontage* WeaponAttacksMontage = nullptr;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UFUNCTION()
	void EquipMeleeWeapon(TSubclassOf<ATimberWeaponBase> WeaponClassName);

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;
};
