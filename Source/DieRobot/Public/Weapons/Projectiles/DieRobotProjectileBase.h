// Property of Paracosm Industries. Dont use my shit.

#pragma once

#include "CoreMinimal.h"
#include "Character/DieRobotPlayableCharacter.h"
#include "GameFramework/Actor.h"
#include "DieRobotProjectileBase.generated.h"

class UCapsuleComponent;
class UProjectileMovementComponent;

UCLASS()
class DIEROBOT_API ADieRobotProjectileBase : public AActor
{
	GENERATED_BODY()

public:
	ADieRobotProjectileBase();

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	UStaticMeshComponent* StaticMesh;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	UCapsuleComponent* CapsuleComponent;

protected:
	virtual void BeginPlay() override;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	ADieRobotPlayableCharacter* ProjectileOwner;
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Projectile Combat Info")
	float ProjectileBaseDamage = 10.f;
	
	UFUNCTION()
	void HandleDestroyAfterNoCollision();

public:
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category="Weapon Projectiles")
	UProjectileMovementComponent* ProjectileMovementComponent;
};
