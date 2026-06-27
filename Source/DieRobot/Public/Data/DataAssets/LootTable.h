// Property of Paracosm.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Loot/EnemyLootDropBase.h"
#include "LootTable.generated.h"

USTRUCT(BlueprintType)
struct FLootEntry
{

	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Loot")
	TSubclassOf<AEnemyLootDropBase> LootItemClass;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Loot")
	TMap<int, float> AmountChance;

	/*There is still a chance of no drop at all.*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Loot")
	float DropChance = 0.0f;

	int32 PickAmount() const
	{
		float Roll = FMath::FRand();
		
		float AccumulatedChance = 0.0f;

		for (const TPair<int32, float>& Pair: AmountChance)
		{
			AccumulatedChance += Pair.Value;
			if (Roll <= AccumulatedChance)
			{
				return Pair.Key;
			}
		}

		return 0; // Won't Happen as all chance values add up to 1.
	}
};

/**
 * 
 */
UCLASS(BlueprintType)
class DIEROBOT_API ULootTable : public UDataAsset
{
	GENERATED_BODY()

public:

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Loot")
	TArray<FLootEntry> LootEntries;
	
};
