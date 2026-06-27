// Property of Paracosm Industries.

#pragma once

#include "CoreMinimal.h"
#include "InputActionValue.h"
#include "BuildSystem/BuildableBase.h"
#include "Character/DieRobotCharacterBase.h"
#include "Interfaces/DamageableEnemy.h"
#include "DieRobotEnemyCharacter.generated.h"

struct FMissionEventPayload;
class AFloatingDataContainer;
class UMetaSoundSource;
class UNavigationHelperComponent;
class UWidgetComponent;
class UStatusEffectHandlerComponent;
class ULootTable;
class ALootHealthDrop;
class AEnemyLootDropBase;
class ADieRobotBuildingComponentBase;
class USoundCue;

UENUM(BlueprintType)
enum class EEnemyWeaponState : uint8
{
	NoWeaponEquipped UMETA(DisplayName = "NoWeaponEquipped"),
	MeleeWeaponEquipped UMETA(DisplayName = "MeleeWeaponEquipped"),
	RangedWeaponEquipped UMETA(DisplayName = "RangedWeaponEquipped"),
};

UENUM(BlueprintType)
enum class EEnemyType : uint8
{
	BasicRobot UMETA(DisplayName = "BasicRobot"),
	MeleeWeaponRobot UMETA(DisplayName = "MeleeWeaponRobot"),
	RangedWeaponRobot UMETA(DisplayName = "RangedWeaponRobot"),
	Drones UMETA(DisplayName = "Drones"),
	Default UMETA(DisplayName = "Default")
};

/**
 * 
 */
UCLASS()
class DIEROBOT_API ADieRobotEnemyCharacter : public ADieRobotCharacterBase, public IDamageableEnemy
{
	GENERATED_BODY()

public:
	ADieRobotEnemyCharacter();
	
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void TakeDamage(FDamagePayload DamagePayload) override;
	
	UFUNCTION()
	void HandleOnMovementModeChanged(class ACharacter* Character, EMovementMode PrevMovementMode, uint8 PreviousCustomMode);

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = Damage)
	UStatusEffectHandlerComponent* StatusEffectHandler;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Navigation")
	UNavigationHelperComponent* NavHelperComponent;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Character Movement")
	UCharacterMovementComponent* CharacterMovementComponent;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Weapon State")
	EEnemyWeaponState EnemyWeaponType = EEnemyWeaponState::NoWeaponEquipped;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Enemy Type")
	EEnemyType EnemyType = EEnemyType::Default;
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Sounds")
	UMetaSoundSource* ProjectileHitSound_MetaSound;
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Sounds")
	UMetaSoundSource* MeleeHitSound_MetaSound;
	
	virtual void PlayProjectileHitSound(FHitResult HitResult) override;
	virtual void PlayMeleeWeaponHitSound(FHitResult HitResult) override;
	
	void SetupCharacterMovementData();
	
	/*Base Data*/
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Base Data")
	float MaxWalkSpeedBase = 400.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Base Data")
	float StandardMelleAttackDamage = 50.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Combat")
	bool bHasBeenAggroByPlayer = false;
	
	/* Montages */
	//TODO:: These can potentially be moved to the Characters Anim Instance. 
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Animation")
	UAnimMontage* DeathMontage;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Animation")
	UAnimMontage* StandardAttackMontage;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Animation")
	UAnimMontage* HitReactMontage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Animation")
	UAnimMontage* SelfDestructMontage;
	
	UFUNCTION(BlueprintCallable)
	void HandleEnemyDeath();
	
	void HandleWeaponDestruction();
	
	/*Sweep / Traces */
	UFUNCTION(BlueprintCallable)
	void SweepForActor(TSubclassOf<AActor> ActorToSweepFor, float SphereRadius = 100.0f, float DistanceToCheck = 100.0f);
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Sweep")
	AActor* SweepHitActor;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sweep")
	float SweepDistance = 250.f;

	UFUNCTION(BlueprintCallable)
	ADieRobotBuildingComponentBase* LineTraceToSeeda();

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Target")
	AActor* CurrentTarget = nullptr;

	UFUNCTION()
	void SelfDestruct();
	void ClearFloatingDamageRef();

	/*UI*/
	void SpawnEffectNameUI(FName EffectName, UStatusEffectBase* StatusEffect);
	
	UFUNCTION()
	void HandleToggleDataView(FInputActionValue Input);
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Data Cluster")
	UWidgetComponent* DataClusterWidgetComponent;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage Effects")
	TObjectPtr<USceneComponent> FloatingDataSpawnLocation;
	
protected:
	virtual void Tick(float DeltaSeconds) override;
	virtual float CalculateOutputDamage(float Damage);
	void SpawnDamageUI(FDamagePayload DamagePayload);
	void StopAiControllerBehaviorTree();
	void OnDeath_HandleCollision();
	void PlayMontageAtRandomSection(UAnimMontage* Montage);
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage")
	TSubclassOf<AFloatingDataContainer> FloatingDamageContainerClass;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Damage")
	TObjectPtr<AFloatingDataContainer> FloatingDamageContainerActor;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Enemy Components")
	USceneComponent* RaycastStartPoint;
	
	/* Damage effects Aggro */
	FTimerHandle DamageWindowTimerHandle;
	float DamageWindowTime = 3.0f;
	float DamageAccumulatedDuringWindow = 0.f;
	void ResetDamageWindow();
	void StartDamageTimerWindow();
	bool HandleAggroCheck(AActor* DamageInstigator, float DamageReceived, float DamageAccumulatedDuringWindow);

	/* Loot */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Loot")
	ULootTable* LootTable = nullptr;
	void OnDeath_DropLoot();

private:
	UFUNCTION()
	void HandleOnLanded(const FHitResult& Hit);
	void ScaleHealth();
	void HandleRemoveStatusEffectComponent();
	void SetupCharacterMovementDelegates();
	void SetupDataClusterComponent();
	
	FMissionEventPayload GenerateDamageEventPayload(FDamagePayload DamagePayload);
	FMissionEventPayload GenerateDestroyEventPayload(FDamagePayload DamagePayload);
	void PopulateCombatEventContextTags(FGameplayTagContainer& ContextTagsContainer, const FDamagePayload& DamagePayload) const;
};
