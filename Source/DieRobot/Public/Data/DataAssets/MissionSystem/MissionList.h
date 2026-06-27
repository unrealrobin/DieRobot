// Property of Paracosm.

#pragma once

#include "CoreMinimal.h"
#include "MissionBase.h"
#include "Engine/DataAsset.h"
#include "MissionList.generated.h"

/**
 * 
 */
UCLASS()
class DIEROBOT_API UMissionList : public UDataAsset
{
	GENERATED_BODY()

public:

	//List of All Possible Missions
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Missions List")
	TArray<UMissionBase*> Missions;
};
