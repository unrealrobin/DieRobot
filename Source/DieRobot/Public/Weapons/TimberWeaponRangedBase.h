// Property of Paracosm Industries.

#pragma once

#include "CoreMinimal.h"
#include "Components/Combat/CombatComponent.h"
#include "Weapons/TimberWeaponBase.h"
#include "TimberWeaponRangedBase.generated.h"

class ATimberCharacterBase;
class USoundCue;

UCLASS()
class DIEROBOT_API ATimberWeaponRangedBase : public ATimberWeaponBase
{
	GENERATED_BODY()

public:
	ATimberWeaponRangedBase();
	
	virtual void Tick(float DeltaTime) override;
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Sounds")
	USoundCue* FiringSound;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Weapon Components")
	USceneComponent* ProjectileSpawnComponent;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Weapon Components")
	TSubclassOf<ATimberProjectileBase> ProjectileType;

protected:
	
	virtual void BeginPlay() override;
	
	FTimerHandle TimeBetweenShotsHandle;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ammo")
	float TimeBetweenProjectiles = 0.1f;

	UFUNCTION()
	void ResetFiringCooldown();

public:
	UPROPERTY(VisibleAnywhere, Category = Owner)
	AActor* WeaponOwner;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Ammo")
	int CurrentAmmo = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ammo")
	int MaxAmmo = 20;
	
	void HandleFiringRate(float InTime);
	/*
	 * Unit offset from Direct Center in All Axis
	 * 1 is 100% Accuracy
	 * Ex. 5 would be +5 and -5 in all axes, and a Random Number taken from that range.
	 * Large is less accurate, small is more accurate.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon Accuract")
	int AIWeaponAccuracy = 20;

	//TODO:: Functionality Moved to Ability & Combat System.
	UFUNCTION()
	void FireRangedWeapon(FVector TargetLocation);

	UFUNCTION(BlueprintCallable)
	void AI_FireRangedWeapon();
	
	/*Reloading*/
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category="Weapon")
	bool bIsReloading = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Ammo")
	bool bIsPowerWeaponCooldown = false;

	//Default false, shot turns to true, timer turns back to false.
	bool bIsFireOnCooldown = false;

	UFUNCTION(BlueprintCallable, Category="Weapon")
	void ReloadWeapon();

	UFUNCTION()
	void PlayReloadMontage();

	UFUNCTION()
	void HandleReloadMontageInterruption(UAnimMontage* Montage, bool bInterrupted);
	
};
