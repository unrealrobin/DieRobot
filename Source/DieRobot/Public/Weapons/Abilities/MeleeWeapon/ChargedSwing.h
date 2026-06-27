// Property of Paracosm.

#pragma once

#include "CoreMinimal.h"
#include "Weapons/Abilities/WeaponAbilityBase.h"
#include "ChargedSwing.generated.h"

UENUM(Blueprintable)
enum class EMontageStage: uint8
{
	None,
	WindUp,
	Loop,
	Final
};

/**
 * 
 */
UCLASS()
class DIEROBOT_API UChargedSwing : public UWeaponAbilityBase
{
	GENERATED_BODY()
	
public:
	
	UChargedSwing();

	virtual void Execute(FAbilityContext Context) override;
	virtual void Execute_Completed(FAbilityContext Context) override;
	virtual void Execute_Cancelled(FAbilityContext Context) override;
	
	void HandleIncompleteAbility(FAbilityContext Context);

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Ability Data")
	float ChargeTimer = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ability Data")
	float FullChargeTimer = 2.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ability Data")
	float DamageDistance = 200.0f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ability Data")
	float CollisionSphereRadius = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ability Data")
	float PerEnemyHitDamage = 20.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Ability Data")
	EMontageStage CurrentMontageStage = EMontageStage::None;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Ability Data")
	TArray<AActor*> ActorsToIgnore;

	virtual void HandleMontageEnded(UAnimMontage* Montage, bool bInterrupted) override;

	UFUNCTION()
	void HandleCollisionSphereBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep,
	                                       const FHitResult& SweepResult);
	UFUNCTION(BlueprintCallable)
	void HandleDamage(FAbilityContext Context);

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Ability Data")
	FAbilityContext AbilityContext;
protected:

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ability Data")
	UAnimMontage* ChargedSwingMontage = nullptr;
	
};
