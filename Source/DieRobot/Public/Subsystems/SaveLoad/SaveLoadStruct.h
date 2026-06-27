// Property of Paracosm Industries.

#pragma once

#include "CoreMinimal.h"
#include "BuildSystem/BuildingComponents/DieRobotBuildingComponentBase.h"
#include "Components/Inventory/InventoryObject.h"
#include "GameFramework/SaveGame.h"
#include "SaveLoadStruct.generated.h"

USTRUCT(BlueprintType)
struct FInventoryData
{
	GENERATED_BODY()
	
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	int NumberOfParts;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	int NumberOfMechanism;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	int NumberOfUniques;

	FInventoryData() :
		NumberOfParts(10), NumberOfMechanism(10), NumberOfUniques(4)
	{}
};

USTRUCT(BlueprintType)
struct FBuildableData
{
	GENERATED_BODY()


	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	TSubclassOf<ABuildableBase> BuildingComponentClass;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	FTransform BuildingComponentTransform;
	
	UPROPERTY(BlueprintReadWrite)
	FGuid GUID;

	UPROPERTY()
	TArray<FGuid> AttachedBuildablesArray;

	UPROPERTY()
	FGuid FrontCenterAttachmentGUID;
	UPROPERTY()
	FGuid BackCenterAttachmentGUID;
	UPROPERTY()
	FGuid FrontTopAttachmentGUID;
	UPROPERTY()
	FGuid FrontBottomAttachmentGUID;
	UPROPERTY()
	FGuid FrontRightAttachmentGUID;
	UPROPERTY()
	FGuid FrontLeftAttachmentGUID;
	UPROPERTY()
	FGuid BackTopAttachmentGUID;
	UPROPERTY()
	FGuid BackBottomAttachmentGUID;
	UPROPERTY()
	FGuid BackRightAttachmentGUID;
	UPROPERTY()
	FGuid BackLeftAttachmentGUID;

	/*Used for Traps/Constructs/Teleporters*/
	UPROPERTY()
	FGuid ParentBuildableGUID;

	/*Used for Deletions of Traps.*/
	UPROPERTY()
	EBuildingComponentTrapDirection TrapDirection = EBuildingComponentTrapDirection::Default;

	/*Teleporter Pairing*/
	UPROPERTY()
	FGuid TeleportPairGUID;

	//Default Constructor
	FBuildableData() :
		BuildingComponentClass(nullptr), BuildingComponentTransform(FTransform::Identity)
	{}

	//Whether or not this Buildable should Override Walkable Slop to "UnWalkable" - Needed for Traps on Vertical Walls.
	UPROPERTY()
	bool bSetIsWalkable = true;
};

USTRUCT(BlueprintType)
struct FPlayerData
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	FVector PlayerLocation;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	FRotator PlayerRotation;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	FInventoryData PlayerInventory;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	TArray<FGuid> CompletedMissionList;

	FPlayerData() :
		PlayerLocation(FVector::ZeroVector), PlayerRotation(FRotator::ZeroRotator), PlayerInventory(FInventoryData())
	{}
	
};

USTRUCT(BlueprintType)
struct FSeedaData
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	FVector SeedaLocation;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	FRotator SeedaRotation;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	float SeedaHealth = 100.f;

	FSeedaData() :
		SeedaLocation(FVector::ZeroVector), SeedaRotation(FRotator::ZeroRotator)
	{}
};

/*
 *
 * Things that should be Saved.
 * 1. Player Location.
 * 2. All Building Components.
 * 2a. Building Component Durability?
 * 3. All of Players Inventory.
 * 4. Wave Start State
 * 4a. Wave Number, Door Positions, All Array contents Empty.
 * 
 */

/**
 * 
 */
UCLASS()
class DIEROBOT_API USaveLoadStruct : public USaveGame
{
	GENERATED_BODY()

public:
	//Save Game Variables Here.

	UPROPERTY(VisibleAnywhere, Category="Wave Data")
	int WaveNumber = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category="Building Component Save Data")
	TArray<FBuildableData> BuildingComponentsArray;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category="Player Save Data")
	FPlayerData PlayerData;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category="Seeda Save Data")
	FSeedaData SeedaData;
};
