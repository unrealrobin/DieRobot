// Property of Paracosm Industries.

#pragma once

#include "CoreMinimal.h"
#include "BuildSystem/BuildableBase.h"
#include "Engine/DataAsset.h"
#include "BuildComponentDataAsset.generated.h"

UENUM(BlueprintType) // Allows usage in Blueprints
enum class EBuildableClassification : uint8
{
	Structure UMETA(DisplayName = "Structure"), // Enum value with display name
	Trap UMETA(DisplayName = "Trap"),
	Construct UMETA(DisplayName = "Construct"),
	Default UMETA(DisplayName = "Default")
};
/**
 * 
 */
UCLASS()
class DIEROBOT_API UBuildComponentDataAsset : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EBuildableClassification BuildableClassification = EBuildableClassification::Default;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSubclassOf<ABuildableBase> BuildingComponentClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int SortValue = 0;
};
