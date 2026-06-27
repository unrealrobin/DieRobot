// Property of Paracosm.

#pragma once

#include "CoreMinimal.h"
#include "TimberEnemyCharacter.h"
#include "EnemyDrone.generated.h"

class USplineComponent;
class AEnemyDroneSplinePath;
class UNiagaraComponent;
class ATimberEnemyProjectile;

UCLASS()
class DIEROBOT_API AEnemyDrone : public ATimberEnemyCharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	AEnemyDrone();
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Components")
	USceneComponent* ProjectileSpawnSceneComponent = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Particle System")
	UNiagaraComponent* DroneParticleSystem = nullptr;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Projectile")
	TSubclassOf<ATimberEnemyProjectile> ProjectileClass = nullptr;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
    
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Spline Paths")
	bool bIsDead = false;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Spline Paths")
	bool bShouldMoveAlongSpline = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Spline Paths")
	bool bIsApproachSpline = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spline Paths")
	bool bHasSightOfKip = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spline Paths")
	bool bHasSightOfSeeda = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spline Paths")
	bool bIsOutsideSpawnArea = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Spline Paths")
	FVector ClosestLabDoorExitLocation;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Spline Paths")
	TArray<AActor*> SplinePathActors; // Array to hold all spline paths in the level
	
	void SelectRandomSplinePath();
	
	void GetAllDroneSplinePathActors();

	void MoveAlongSplinePath(float DeltaTime);

	void MoveToOutsideSpawnArea(float DeltaTime);

	void FlyToSplineStart(float DeltaTime);

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Spline Paths")
	float CurrentDistanceAlongSplinePath = 0.0f; // Distance along the spline path

	UFUNCTION()
	virtual void TakeDamage(FDamagePayload DamagePayload) override;

	UFUNCTION()
	void DestroyAfterDelay();
	
	UFUNCTION(BlueprintCallable)
	void GatherTargets();

	FTimerHandle TargetingTimerHandle;

	UFUNCTION(BlueprintCallable)
	void ClearCurrentTarget();
	
	UFUNCTION()
	void ShootTarget();

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Targets")
	TArray<AActor*> TargetActors; // Array to hold all target actors in the level
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spline Path")
	AEnemyDroneSplinePath* SplinePathRef = nullptr; // Reference to the spline path this drone will follow
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spline Path")
	USplineComponent* SplineComponent = nullptr; // Reference to the spline component of the selected spline path

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spline Path")
	float DroneApproachSpeed = 400.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spline Path")
	float DroneFlightSpeed = 300.0f;

private:
	void GetClosestLabDoorExitLocation();

};




