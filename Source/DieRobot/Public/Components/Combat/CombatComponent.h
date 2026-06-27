// Property of Paracosm.

#pragma once

#include "CoreMinimal.h"
#include "InputActionValue.h"
#include "Components/ActorComponent.h"
#include "Weapons/TimberWeaponBase.h"
#include "CombatComponent.generated.h"

enum class EAbilityInputRequirement : uint8;
class UCombatComponent;
struct FInputActionValue;

UENUM(BlueprintType)
enum class EOwnerWeaponState : uint8
{
	Unequipped UMETA(DisplayName = "None"),
	MeleeWeaponEquipped UMETA(DisplayName = "MeleeWeaponEquipped"),
	RangedWeaponEquipped UMETA(DisplayName = "RangedEquipped"),
	Default UMETA(DisplayName = "Default")
};

/*
 *Context used to pass to Weapon Abilities
 */
USTRUCT(BlueprintType)
struct FAbilityContext
{
	GENERATED_BODY()
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Ability Context")
	UCombatComponent* CombatComponent = nullptr;

	//The player/Character that initiated the ability
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Ability Context")
	AActor* Instigator = nullptr;

	//The Weapon that initiated the ability
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Ability Context")
	ATimberWeaponBase* WeaponInstance = nullptr;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Ability Context")
	FVector TargetLocation = FVector::ZeroVector;
	
	FInputActionValue InputActionValue = FInputActionValue();
};

class ATimberPlayableCharacter;
class ATimberWeaponRangedBase;
class ATimberWeaponMeleeBase;

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class DIEROBOT_API UCombatComponent : public UActorComponent
{
	GENERATED_BODY()

public:

	UCombatComponent();
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	/*
	 * Stores the Spawned Weapon Instances
	 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Weapon")
	ATimberWeaponRangedBase* RangedWeaponInstance = nullptr;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Weapon")
	ATimberWeaponMeleeBase* MeleeWeaponInstance = nullptr;

	/*
	 * Stores the currently Equipped Weapon
	 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Ability Context")
	UWeaponAbilityBase* CurrentWeaponAbility = nullptr;

	bool bIsEquipMontagePlaying = false;

	/*
	 * Used to manage time between attacks.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bCanMeleeAttack = true;

	/*
	 *Socket Names Set on SKM of Kip.
	 *Used for attachment slots.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Weapon Sockets")
	FName UnEquippedMeleeSocket = "UnEquippedMeleeSocket";
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Weapon Sockets")
	FName EquippedMeleeSocket = "EquippedMeleeSocket";
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Weapon Sockets")
	FName UnEquippedRangeSocket = "UnEquippedRangeSocket";
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Weapon Sockets")
	FName EquippedRangeSocket = "EquippedRangeSocket";

	/*
	 * Adjusts the projectile direction at X Distance. TPS lining up of projectile.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Projectile Targeting")
	float ProjectileAlignmentAdjustmentDistance = 500.0f;
	
	/*
	 * Spawns the Weapons
	 */
	void SpawnRangedWeapon();
	void SpawnMeleeWeapon();
	
	/*
	 * Called from Animation Notifies
	 */
	UFUNCTION(BlueprintCallable)
	void EquipMelee();
	UFUNCTION(BlueprintCallable)
	void EquipRanged();
	
	void UnEquipAllWeapons();
	void UnEquipCurrentlyEquippedWeapon();

	void PlayEquipWeaponMontage(FName MontageSectionName);

	/* Reloads Ammo-Based Weapons*/
	void ReloadRangedWeapon();

	//Pressed Trigger Event Abilities
	void HandlePrimaryAbility(const FInputActionValue& Value);
	void HandleSecondaryAbility(const FInputActionValue& Value);
	
	/*
	 * Callbacks Used for Stage-Based Animations.
	 * Ex. Hold for X Duration to build energy, then fire.
	 */
	void HandleSecondaryAbility_Started(const FInputActionValue& Value);
	void HandleSecondaryAbility_Cancelled(const FInputActionValue& Value);
	void HandleSecondaryAbility_Completed(const FInputActionValue& Value);
	
	/*
	 * Animation Montage Helpers
	 */
	void PlayCharacterAnimationMontage(UAnimMontage* Montage, FName MontageSectionName, float PlayRate = 1.0f, bool TrackStages = false);
	void StopCharacterAnimationMontage(UAnimMontage* Montage, float BlendOutTime);

	/*
	 * Callback used based on animation montage completion.
	 */
	UFUNCTION()
	void HandleMontageEnded(UAnimMontage* Montage, bool bInterrupted);
	
	/*
	 * Used for checking what type of Input (Pressed, Released, Held) is required for the ability.
	 */
	EAbilityInputRequirement GetAbilityInputRequirement(bool bIsPrimaryAbility) const;
	
	/*
	 * Getters
	 */
	FORCEINLINE ATimberWeaponBase* GetCurrentlyEquippedWeapon() const { return CurrentlyEquippedWeapon; }
	FORCEINLINE EOwnerWeaponState GetCurrentWeaponState() const { return CurrentWeaponState; }

	/*
	 * Used to generate Context for the Weapon Abilities
	 */
	FVector GetProjectileTargetLocation();

protected:

	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Data")
	ACharacter* OwningCharacter = nullptr;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Data")
	ATimberWeaponBase* CurrentlyEquippedWeapon = nullptr;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Data")
	EOwnerWeaponState CurrentWeaponState = EOwnerWeaponState::Unequipped;
	
	/* Weapon Classes (For Spawning)*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Weapon")
	TSubclassOf<ATimberWeaponBase> RangedWeaponClass;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Weapon")
	TSubclassOf<ATimberWeaponBase> MeleeWeaponClass;
	
	void EquipWeapon(ATimberWeaponBase* WeaponInstance, FName EquippedWeaponSocketName);
	void UnEquipWeapon(ATimberWeaponBase* WeaponInstance, FName UnEquipSocketName);

	/*
	 * Validates if the ability can be fired based on Cooldowns and Resources
	 */
	bool ValidateNoResourceCostAbility(const UWeaponAbilityBase* WeaponAbilityBase);
	bool ValidateWeaponAbility(const UWeaponAbilityBase* AbilityToValidate);
	
private:

	UPROPERTY()
	bool bIsRifleEquipped = false;
	
	UPROPERTY()
	bool bIsMeleeEquipped = false;
	
	/*
	 * Generates the context needed for Abilities to have all their logic encapsulated.
	 */
	FAbilityContext GenerateCurrentAbilityContext(const FInputActionValue& InputValue);
	
	UFUNCTION()
	void SpawnWeaponAtSocketLocation(TSubclassOf<ATimberWeaponBase> WeaponClassToSpawn, FName SocketName);
	
	UFUNCTION()
	void SendWeaponStateToOwnerAnimInstance();
	
	/*
	 * Whether the character weapon ability is affordable based on Weapon available power
	 */
	bool bHasEnoughPower(float AbilityCost, float CurrentWeaponPower);

	/*
	 * Consumes Power to Execute Ability
	 */
	void ConsumePower(ATimberWeaponBase* WeaponInstance, float AmountToConsume);

	/*
	 * Checks whether the ability is a Power based ability (Ex. Not ammo based.)
	 */
	bool ValidatePowerWeaponAbility(const UWeaponAbilityBase* AbilityToValidate);
	
	void UpdateCurrentWeaponState(EOwnerWeaponState NewWeaponState);
	
};
