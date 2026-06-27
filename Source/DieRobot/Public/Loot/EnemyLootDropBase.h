// Property of Paracosm Industries.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "EnemyLootDropBase.generated.h"

class UCapsuleComponent;

UCLASS()
class DIEROBOT_API AEnemyLootDropBase : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this pawn's properties
	AEnemyLootDropBase();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	void SetCollisionProperties();

	virtual void PlaySFX();

	UFUNCTION()
	virtual void HandleLootItemOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, 
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult &SweepResult);

	UFUNCTION()
	void NoPickupAutoDestroy();


public:

	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category="Component Details")
	UStaticMeshComponent* StaticMesh;

	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category="Component Details")
	UCapsuleComponent* CollisionCapsule;

	

};
