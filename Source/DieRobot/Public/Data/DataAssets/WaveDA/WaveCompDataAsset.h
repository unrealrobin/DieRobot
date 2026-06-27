// Property of Paracosm.

#pragma once

#include "CoreMinimal.h"
#include "Character/Enemies/DieRobotEnemyCharacter.h"
#include "Engine/DataAsset.h"
#include "WaveCompDataAsset.generated.h"

/**
 * 
 */
UCLASS()
class DIEROBOT_API UWaveCompDataAsset : public UDataAsset
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wave Composition")
	TSubclassOf<ADieRobotEnemyCharacter> BasicRobotClass;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wave Composition")
	TSubclassOf<ADieRobotEnemyCharacter> MeleeRobotClass;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wave Composition")
	TSubclassOf<ADieRobotEnemyCharacter> RangedRobotClass;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wave Composition")
	TSubclassOf<ADieRobotEnemyCharacter> DroneClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wave Composition")
	UCurveTable* SpawningCurveTable;
};
