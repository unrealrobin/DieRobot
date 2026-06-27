// Property of Paracosm Industries. Dont use my shit.

#pragma once

#include "CoreMinimal.h"
#include "EnemyLootDropBase.h"
#include "LootHealthDrop.generated.h"

UCLASS()
class DIEROBOT_API ALootHealthDrop : public AEnemyLootDropBase
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	ALootHealthDrop();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UFUNCTION()
	virtual void HandlePlayerOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* 
	OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult & SweepResult);

public:
	
	virtual void Tick(float DeltaTime) override;

	FTimerHandle HealthAvailableTimerHandle;

	UFUNCTION()
	void HandleDestroy();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Health")
	float HealthGainAmount = 20.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Health")
	float HealthAvailableTime = 20.f;
};


