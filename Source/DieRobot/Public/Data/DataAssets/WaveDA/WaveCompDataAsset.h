// Property of Paracosm.

#pragma once

#include "CoreMinimal.h"
#include "Character/Enemies/TimberEnemyCharacter.h"
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
	TSubclassOf<ATimberEnemyCharacter> BasicRobotClass;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wave Composition")
	TSubclassOf<ATimberEnemyCharacter> MeleeRobotClass;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wave Composition")
	TSubclassOf<ATimberEnemyCharacter> RangedRobotClass;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wave Composition")
	TSubclassOf<ATimberEnemyCharacter> DroneClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wave Composition")
	UCurveTable* SpawningCurveTable;
};
