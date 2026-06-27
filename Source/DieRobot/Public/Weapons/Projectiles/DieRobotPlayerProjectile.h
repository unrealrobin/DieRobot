// Property of Paracosm Industries.

#pragma once

#include "CoreMinimal.h"
#include "DieRobotProjectileBase.h"
#include "DieRobotPlayerProjectile.generated.h"

UCLASS()
class DIEROBOT_API ADieRobotPlayerProjectile : public ADieRobotProjectileBase
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	ADieRobotPlayerProjectile();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UFUNCTION()
	void HandleBlocked(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);

	UFUNCTION()
	float CalculateOutputDamage(ADieRobotWeaponRangedBase* Weapon);

	//Set in Deferred spawn on Weapon.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Ownership")
	ADieRobotPlayableCharacter* PlayerProjectileOwner;
	
private:
	
	UFUNCTION()
	void HandleDestroy();
};



