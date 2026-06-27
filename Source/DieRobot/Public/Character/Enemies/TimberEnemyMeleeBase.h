// Property of Paracosm Industries.

#pragma once

#include "CoreMinimal.h"
#include "Character/Enemies/TimberEnemyCharacter.h"
#include "TimberEnemyMeleeBase.generated.h"

/**
 * 
 */
UCLASS()
class DIEROBOT_API ATimberEnemyMeleeBase : public ATimberEnemyCharacter
{
	GENERATED_BODY()

public:
	ATimberEnemyMeleeBase();

	virtual void BeginPlay() override;

	/*Overlap Delegate*/
	UFUNCTION()
	void HandleCapsuleOverlap(
		UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	/* Melee Capsule Components */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Melee Components")
	UCapsuleComponent* RightHandCapsuleComponent;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Melee Components")
	UCapsuleComponent* LeftHandCapsuleComponent;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Melee Components")
	UCapsuleComponent* RightFootCapsuleComponent;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Melee Components")
	UCapsuleComponent* LeftFootCapsuleComponent;

	/* Called in Animation Notifies*/
	UFUNCTION(BlueprintCallable)
	void EnableCapsuleComponent(UCapsuleComponent* MeleeCapsuleComponent);
	UFUNCTION(BlueprintCallable)
	void DisableCapsuleComponent(UCapsuleComponent* MeleeCapsuleComponent);

	/* Attack Information*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float BaseMeleeAttackDamage = 20.0f;
	UFUNCTION()
	virtual float CalculateOutputDamage(float Damage) override;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Attack Information")
	TArray<AActor*> ActorsToIgnore;
};
