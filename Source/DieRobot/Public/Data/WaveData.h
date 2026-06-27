#pragma once

#include "CoreMinimal.h"
#include "Data/DataAssets/EnemySpawnDataAsset.h"
#include "WaveData.generated.h"

USTRUCT(BlueprintType)
struct FEnemyData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy Data")
	UEnemySpawnDataAsset* EnemyDataAsset;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy Data")
	int32 NumberOfEnemiesToSpawn;

	FEnemyData()
	{
		EnemyDataAsset = nullptr;
		NumberOfEnemiesToSpawn = 0;
	}
	
};

USTRUCT(BlueprintType)
struct FWaveCompositionRow : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wave Composition Data")
	int32 WaveNumber;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wave Composition Data")
	TArray<FEnemyData> EnemiesToSpawnArray;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wave Composition Data")
	TSubclassOf<AActor> BossEnemy;

	FWaveCompositionRow()
	{
		WaveNumber = 0;
		BossEnemy = nullptr;
	}
	
};

