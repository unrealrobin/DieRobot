// Property of Paracosm Industries. 

#pragma once

#include "CoreMinimal.h"
#include "SaveLoadStruct.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Types/DieRobotGlobalSaveDataStruct.h"
#include "SaveLoadSubsystem.generated.h"

class ATimberGameModeBase;
class UDieRobotGlobalSaveData;
/**
 * 
 */
UCLASS()
class DIEROBOT_API USaveLoadSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	UPROPERTY()
	TMap<FGuid, ABuildableBase*> GuidToBuildableMap;
	UClass* SeedaClass = nullptr;

	UFUNCTION(BlueprintCallable)
	void SetNewGameSaveSlot();
	UFUNCTION(BlueprintCallable)
	void SetLoadGameSaveSlot(FString SlotName);
	UFUNCTION(BlueprintCallable)
	void SetLastPlayedSaveSlot();
	
	/* Save System */
	UFUNCTION(BlueprintCallable, Category="Save System")
	void SaveCurrentGame();

	/* Load System */
	UFUNCTION(BlueprintCallable, Category="Save System")
	void LoadGame(FString SlotToLoad);

	UFUNCTION(BlueprintCallable)
	FORCEINLINE FString GetCurrentSessionSaveSlot() { return CurrentSessionSaveSlot; }

	/* Retrieves the Global Save Data from the GLOBAL_SAVE_DATA file*/
	UDieRobotGlobalSaveData* GetGlobalSaveDataInstance();
private:

	UPROPERTY()
	TArray<FString> SaveSlots;
	UPROPERTY()
	FString CurrentSessionSaveSlot;
	UPROPERTY()
	FString GlobalSaveDataSlotName = "GLOBAL_SAVE_DATA";

	void SetCurrentSessionSaveSlot(FString SlotName);
	FString GetLastPlayedSaveSlot();
	
	void SaveBuildableData(USaveLoadStruct* SaveGameInstance);
	void SaveWaveData(USaveLoadStruct* SaveGameInstance);
	void SavePlayerData(USaveLoadStruct* SaveGameInstance);
	void SaveSeedaData(USaveLoadStruct* SaveGameInstance);
	
	void LoadBuildableData(USaveLoadStruct* LoadGameInstance);
	void LoadWaveData(USaveLoadStruct* LoadGameInstance);
	void LoadPlayerState(USaveLoadStruct* LoadGameInstance);
	void LoadSeedaData(USaveLoadStruct* LoadGameInstance);

	/* Utils */
	// Deferring the Loading of the Player State to After the Initialization of the Character on Level Load.
	UFUNCTION()
	void OnCharacterInitialization();
	void CheckBuildingComponentForSnapAttachments(FBuildableData& BuildableData, ATimberBuildingComponentBase* BuildingComponent);
	void RemoveAllLabBuildables();
	void RemoveAllLootItems();
	//Deferred Character Initialization Bindings
	void BindToGameModeBaseDelegate(ATimberGameModeBase* GameModeBase);
	//Register Buildables - Add to TMap
	void RegisterBuildable(ABuildableBase* Buildable);
	//DeRegister Buildables - Remove Buildables from TMap
	void DeRegisterBuildable(FGuid BuildableGUID);
	//Move all Save and Load functionality here.
	bool bIsBuildableRegistered(FGuid BuildableGUID);
	//Resolving Linking of Parents/Pairs/Attached Buildables.
	void ResolveBuildableReferences(TArray<FBuildableData> BuildableData);

	/* Global Save Data Utils */
	
	/* Generates an up to Data Save Slot Struct that lives in the ActiveSaveSlots.*/
	FSaveSlotDataStruct GenerateSaveSlotDataStruct(FString SlotName);
	
	void AddNewSaveSlotToGlobalSaveSlotList();
	void SaveSessionDataToGlobalSaveSlotList();
	
};

