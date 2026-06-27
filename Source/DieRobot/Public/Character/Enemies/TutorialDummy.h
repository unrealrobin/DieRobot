// Property of Paracosm Industries. Dont use my shit.

#pragma once

#include "CoreMinimal.h"
#include "TimberEnemyCharacter.h"
#include "TutorialDummy.generated.h"

class ADieRobotGameStateBase;

UCLASS()
class DIEROBOT_API ATutorialDummy : public ATimberEnemyCharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	ATutorialDummy();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collision")
	UCapsuleComponent* CollisionCapsule = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Components")
	USceneComponent* LootDropLocation;

	virtual void TakeDamage(FDamagePayload DamagePayload) override;

	UFUNCTION()
	void HandleDeath(AActor* DeadActor);

	UPROPERTY()
	ADieRobotGameStateBase* DieRobotGameStateBase;
	
};
