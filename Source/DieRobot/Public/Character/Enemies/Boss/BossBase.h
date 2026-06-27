// Property of Paracosm Industries.

#pragma once

#include "CoreMinimal.h"
#include "Character/Enemies/TimberEnemyCharacter.h"
#include "BossBase.generated.h"

UCLASS()
class DIEROBOT_API ABossBase : public ATimberEnemyCharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	ABossBase();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss Info")
	FName BossTechnicalName = "SET_BOSS_TECHNICAL_NAME";

	/*Delegates*/
	DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnBossDeath);
	FOnBossDeath OnBossDeath;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	//virtual void HandleDropHealthLoot(TSubclassOf<AEnemyLootDropBase> HealthDropClass) override;

	UFUNCTION()
	void BroadcastDeathDelegate(AActor* DestroyedActor);

	
	
};
