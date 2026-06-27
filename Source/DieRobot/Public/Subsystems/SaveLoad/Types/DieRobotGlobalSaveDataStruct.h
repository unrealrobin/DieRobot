#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "DieRobotGlobalSaveDataStruct.generated.h"
// Used to Save Global Data for Player.

USTRUCT(BlueprintType, meta=(scriptName="GlobalSaveDataStruct"))
struct FDieRobotGlobalSaveDataStruct
{
	GENERATED_BODY()

	UPROPERTY()
	FString LastSavedGame;
};

USTRUCT(BlueprintType, meta=(scriptName="GlobalSaveDataStruct"))
struct FSaveSlotDataStruct
{
	GENERATED_BODY()

	UPROPERTY()
	FString SlotName;

	UPROPERTY()
	FGuid SlotGuid;

	UPROPERTY()
	int SlotCurrentWave = 0;

	UPROPERTY();
	FString LastTimeStamp;
};

UCLASS()
class DIEROBOT_API UDieRobotGlobalSaveData : public USaveGame
{
	GENERATED_BODY()

public:
	UPROPERTY()
	FDieRobotGlobalSaveDataStruct LastSavedSlot;

	UPROPERTY()
	TArray<FSaveSlotDataStruct> ActiveSaveSlots;

	UFUNCTION()
	FString GetCurrentTimeStamp();
};

