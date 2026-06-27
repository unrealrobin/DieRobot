// Property of Paracosm.

#pragma once

#include "CoreMinimal.h"
#include "Weapons/Abilities/WeaponAbilityBase.h"
#include "KnockbackBlast.generated.h"

/**
 * 
 */
UCLASS()
class DIEROBOT_API UKnockbackBlast : public UWeaponAbilityBase
{
	GENERATED_BODY()

public:
	UKnockbackBlast();

	UPROPERTY()
	UBoxComponent* HitBox = nullptr;

	UFUNCTION()
	void HandleOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
	
	virtual void Execute(FAbilityContext Context) override;

protected:

	void MoveBoxForward();
	void ScaleBoxExtent();
	void HandleDestructionCheck();
	void IncrementBlastBox();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Knockback Details")
	float ImpulseForce = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Knockback Details")
	float ImpulseForceZ = 0.4f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Knockback Details")
	float KnockbackDuration = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Knockback Details")
	FVector InitialBoxExtent = FVector(20.f, 20.f, 20.f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Knockback Details")
	FVector EndBoxExtent = FVector(20.f, 400.f, 200.f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Knockback Details")
	float CurrentTimeStep = 0;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Knockback Details")
	float EndTimeStep = 10;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Knockback Details")
	FVector InitialBoxLocation = FVector::ZeroVector;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Knockback Details")
	float EndLocationDelta = 400.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Knockback Details")
	FVector EffectVectorOffset = FVector(100.0f, 0.0, 0.0);
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Knockback Details")
	float DamageAmount = 20.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Knockback Details")
	float EnemyStunTime = 1.0f;

private:
	UPROPERTY()
	TArray<AActor*> ActorsToIgnore;
	
	FTimerHandle BlastTimerHandle;

	FAbilityContext LocalContext;
	

};
