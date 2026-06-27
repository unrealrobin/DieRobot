// Property of Paracosm Industries.

#pragma once

#include "CoreMinimal.h"
#include "TimberProjectileBase.h"
#include "TimberEnemyProjectile.generated.h"

UCLASS()
class DIEROBOT_API ATimberEnemyProjectile : public ATimberProjectileBase
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	ATimberEnemyProjectile();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	virtual bool IsActorCurrentTarget(AActor* OtherActor);

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UFUNCTION()
	void HandleBlocked(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);

	UFUNCTION()
	void HandleOverlap(
		UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	
};
