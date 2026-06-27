// Property of Paracosm Industries.	

#pragma once

#include "CoreMinimal.h"
#include "Character/DieRobotCharacterBase.h"
#include "Interfaces/Amplifiable.h"
#include "Interfaces/CombatComponentAnimUser.h"
#include "States/PlayerStateBase.h"
#include "Weapons/DieRobotWeaponBase.h"
#include "DieRobotPlayableCharacter.generated.h"

enum class ETutorialState : uint8;
class UMissionDeliveryComponent;
class UPlayerVignetteComponent;
class UCombatComponent;
class ABuildableBase;
class UInventoryObject;
class ADieRobotPlayerController;
class ADieRobotWeaponRangedBase;
class ADieRobotWeaponMeleeBase;
class UBuildSystemManagerComponent;
class ADieRobotBuildingComponentBase;
class USpringArmComponent;
class UCameraComponent;
class UInventoryManagerComponent;

UENUM(BlueprintType)
enum class ECharacterState: uint8
{
	Standard UMETA(DisplayName = "Standard"),
	Building UMETA(DisplayName = "Building"),
	Amplified UMETA(DisplayName = "Amplified")
};

/**
 * 
 */
UCLASS()
class DIEROBOT_API ADieRobotPlayableCharacter : public ADieRobotCharacterBase, public ICombatComponentAnimUser, public IAmplifiable
{
	GENERATED_BODY()

public:
	ADieRobotPlayableCharacter();

	//Delegates Signatures
	DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnCharacterInitialization);
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FHandlePlayerDeath, bool, bIsPlayerDead);
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FHandleSpawnDeleteIconLocation, float, ViewportLocationX, float, ViewportLocationY);
	DECLARE_DYNAMIC_MULTICAST_DELEGATE(FHandleRemoveDeleteIcon);
	
	//Delegate Handles
	FOnCharacterInitialization OnCharacterInitialization;
	FHandlePlayerDeath HandlePlayerDeath_DelegateHandle;
	FHandleSpawnDeleteIconLocation HandleSpawnDeleteIconLocation_DelegateHandle;
	FHandleRemoveDeleteIcon HandleRemoveDeleteIcon_DelegateHandle;

	/*Interface Overrides*/
	virtual void PlayWeaponEquipAnimationMontage(FName SectionName) override;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Animation Data")
	float EquipWeaponPlayRate = 3.0f;
	
	//IAmplifiable
	virtual void SetIsAmplified(bool bIsAmplified) override;
	
	
	//CharacterState
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Character State")
	ECharacterState CharacterState = ECharacterState::Standard;

	/* Get Game State */
	ETutorialState GetTutorialState();
	
	/*Components*/
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Camera")
	UCameraComponent* Camera;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Camera")
	USpringArmComponent* CameraSpringArm;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Build System")
	UBuildSystemManagerComponent* BuildSystemManager;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Inventory")
	UInventoryManagerComponent* InventoryManager;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Combat Component")
	UCombatComponent* CombatComponent;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Vignette Component")
	UPlayerVignetteComponent* VignetteComponent;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Missions Component")
	UMissionDeliveryComponent* MissionDeliveryComponent;

	UPROPERTY()
	TObjectPtr<UCapsuleComponent> TempAmplifyCapsule;
	UPROPERTY()
	TObjectPtr<UStaticMeshComponent> TempAmplifyStaticMeshComponent;
	UPROPERTY(EditAnywhere,BlueprintReadWrite, Category="Amplify Static Mesh")
	TObjectPtr<UStaticMesh> AmplifyMesh;
	
	/*Attributes / Defaults*/
	bool IsRunning = true;
	float MaxRunSpeed = 2000.f;

	/*Animation Montages*/
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Animation")
	UAnimMontage* DeathMontage;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Animation")
	UAnimMontage* TutorialWakeMontage;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Animation")
	UAnimMontage* TurnInPlaceMontage;
	UFUNCTION()
	void StopAllAnimMontages();
	UFUNCTION()
	void PlayWakeAnimationMontage();
	void PlayAnimationMontageAtSection(UAnimMontage* MontageToPlay, FName SectionName);

	/*Smooth Character Rotation*/
	//Checks if the player is undergoing a Timed Rotation
	bool IsRotating = false;
	//Time Elapsed in-between timer calls.
	float ElapsedTime = 0.0f;
	FTimerHandle RotationTimerHandle;
	//Handles that actual timing and set of the Rotation
	void StartLerpRotation(const FRotator& TargetRotation, float DurationOfRotation);
	
	/*Animation Properties*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Animation")
	bool IsNowJumping = false;
	
	/*Combat*/
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Animation")
	UAnimMontage* ReloadMontage;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Animation")
	UAnimMontage* EquipWeaponMontage;
	
	UFUNCTION()
	void PlayEquipWeaponMontage(FName SectionName);

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<ADieRobotWeaponBase> CurrentlyEquippedWeapon;
	
	void HandleRaycastHitConditions(bool bHits);
	
	ADieRobotWeaponBase* GetCurrentlyEquippedWeapon() const { return CurrentlyEquippedWeapon; }

	/*Build System*/
	void PerformBuildSystemRaycast();
	
	bool HandleShowDeleteWidget();
	
	UFUNCTION()
	void ResetDeleteIcon();
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Player Controller")
	ADieRobotPlayerController* PlayerController;
	
	UPROPERTY(EditAnywhere, Category="Build System Info")
	float BuildRaycastDistance = 2000.f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Build System Info")
	bool bIsTheBuildMenuOpen = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Build System Info")
	ABuildableBase* HoveredBuildingComponent;

	UPROPERTY(VisibleAnywhere, Category="Build System Info")
	TArray<FHitResult> HitResults;
	
	FVector BuildingComponentImpactPoint;

	/*Damage*/
	void PlayerTakeDamage(float DamageAmount);
	
	void PlayerGainHealth(float HealthAmount);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Damage")
	float DamageModifierValue = 1.f;

	/*Death*/
	UFUNCTION()
	void HandlePlayerDeath(bool bIsPlayerDead = false);
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Health")
	bool bIsPlayerDead = false;

	/*Inventory*/
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Inventory")
	UInventoryObject* InventoryObject = nullptr;

	UFUNCTION()
	void GetPlayerInventoryFromPlayerState();

	UFUNCTION()
	void UnbindFromSeedaDeathDelegate(AActor* DestroyedActor);
	
	UFUNCTION()
	void BindToSeedaDelegates(AActor* Seeda);
	
	UFUNCTION(BlueprintCallable)
	FORCEINLINE float GetAmplificationPower(){return AmplificationPower;}
	
	UFUNCTION()
	void SetAmplificationPower(float DeltaPower);

protected:
	virtual void BeginPlay() override;
	
	virtual void Tick(float DeltaSeconds) override;
	
	
	UFUNCTION()
	void HandleWaveEnd(int CompletedWaveNumber);
	

	UFUNCTION()
	void HandleBuildMenuOpen(bool IsBuildMenuOpen);

	UFUNCTION()
	void PlayDeathAnimation();

private:

	UFUNCTION()
	void HandleAmplificationCapsuleOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep,const FHitResult& SweepResult);

	UFUNCTION()
	void HandleAmplificationCapsuleEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);	

	void CreateAmplificationSphere();
	
	//We want to add some form of reducing power here when in amplification, when out of amplified, the resource rebuilds its power.
	float AmplificationPower = 100.0f;
	float NotAmplifiedTime = 0.0f;
	bool bEnoughPowerToAmplify();
	void UsePowerForAmplification();
	void GainPowerForAmplification();
	
};

