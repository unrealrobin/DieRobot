// Property of Paracosm Industries. Dont use my shit.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Interfaces/Interactable.h"
#include "TestObj.generated.h"

class ATimberEnemyCharacter;
class UCapsuleComponent;

UCLASS()
class DIEROBOT_API ATestObj : public AActor, public IInteractable
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	ATestObj();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	virtual void Interact() override;

	UPROPERTY(EditAnywhere)
	TObjectPtr<UCapsuleComponent> CollisionCapsule;
	UPROPERTY(EditAnywhere)
	TObjectPtr<UStaticMeshComponent> StaticMeshBase;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<UStaticMeshComponent> LeverStaticMesh;

	UFUNCTION()
	void SetInteractItem(
		UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
	UFUNCTION()
	void UnSetInteractItem(
		UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex);

	UPROPERTY(EditAnywhere, Category = "Enemy")
	TArray<TSubclassOf<ATimberEnemyCharacter>> EnemyToSpawn;

protected:
	FRotator InitialLeverRotation;
	FRotator LeverOnRotation = FRotator(0.0f, 0.0f, -50.0f);
};
