// Property of Paracosm Industries. Dont use my shit.

#pragma once

#include "CoreMinimal.h"
#include "Character/Enemies/DieRobotEnemyCharacter.h"
#include "Engine/DataAsset.h"
#include "EnemySpawnDataAsset.generated.h"

UENUM(BlueprintType)
enum class EEnemySpawnLocation : uint8
{
	StandardSpawn UMETA(DisplayName = "StandardSpawn"),
	BossSpawn UMETA(DisplayName = "BossSpawn")
};

/**
 * 
 */
UCLASS(BlueprintType)
class DIEROBOT_API UEnemySpawnDataAsset : public UDataAsset
{
	GENERATED_BODY()

public:

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy Data")
	TSubclassOf<ADieRobotEnemyCharacter> EnemyClassReference;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy Data")
	EEnemySpawnLocation SpawnLocation;

	
};
