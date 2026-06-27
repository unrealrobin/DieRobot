// Property of Paracosm.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "MetasoundSource.h"
#include "Engine/DataAsset.h"
#include "MissionBase.generated.h"

UENUM(BlueprintType)
enum class ERewardType: uint8
{
	Currency,
	None,
	Default,
};

USTRUCT(BlueprintType)
struct FCurrencyStruct
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int PartsReward = 0;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int MechanismsReward = 0;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int UniquesReward = 0;
};

/**
 * 
 */
UCLASS()
class DIEROBOT_API UMissionBase : public UDataAsset
{
	GENERATED_BODY()

public:

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission Info")
	FGuid MissionID; 

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission Info")
	FText MissionTitle;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission Info")
	FText MissionDescription;
	//Ex. Kills, Builds, Timer, other..

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission Info")
	FGameplayTag MissionEventType;
	//Events must contain all RequiredContextTags to be countable for this mission.

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission Info")
	FGameplayTagContainer RequiredContextTags;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission Info")
	bool MultipleObjectives = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission Info")
	int NumericMissionGoal = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission Info")
	UMetaSoundSource* MissionDialogue;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission Rewards")
	FCurrencyStruct CurrencyReward;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission Rewards")
	ERewardType RewardType = ERewardType::Default;
};
