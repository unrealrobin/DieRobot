// Property of Paracosm Industries. Dont use my shit.

#pragma once

#include "CoreMinimal.h"
#include "Character/TimberPlayableCharacter.h"
#include "GameFramework/Actor.h"
#include "TimberProjectileBase.generated.h"

class UCapsuleComponent;
class UProjectileMovementComponent;

UCLASS()
class DIEROBOT_API ATimberProjectileBase : public AActor
{
	GENERATED_BODY()

public:
	ATimberProjectileBase();

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	UStaticMeshComponent* StaticMesh;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	UCapsuleComponent* CapsuleComponent;

protected:
	virtual void BeginPlay() override;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	ATimberPlayableCharacter* ProjectileOwner;
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Projectile Combat Info")
	float ProjectileBaseDamage = 10.f;
	
	UFUNCTION()
	void HandleDestroyAfterNoCollision();

public:
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category="Weapon Projectiles")
	UProjectileMovementComponent* ProjectileMovementComponent;
};
