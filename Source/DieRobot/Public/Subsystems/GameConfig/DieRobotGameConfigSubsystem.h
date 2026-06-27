// Property of Paracosm Industries. Dont use my shit.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "DieRobotGameConfigSubsystem.generated.h"

UENUM(BlueprintType)
enum class EDieRobotGameConfigType : uint8
{
	MainMenu UMETA(DisplayName = "MainMenu"),
	MidGameDemo UMETA(DisplayName = "MidGameDemo"),
	Standard UMETA(DisplayName = "Standard"),
	Default UMETA(DisplayName = "Default"),
};

/*GameConfig
 * This Class tells the Game what type of GameMode we are in.
 * Initially this was to distinguish between demos, but it also serves a good purpose to clarify which game mode the player is in.
 * Ex. Story Mode, Endless Mode, Other, Etc.
 * At the Moment it is used to help select the Correct Game Slot to Save/Load to/from.
 */

/**
 * 
 */
UCLASS()
class DIEROBOT_API UDieRobotGameConfigSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	
	UPROPERTY()
	EDieRobotGameConfigType GameConfig = EDieRobotGameConfigType::Standard;
};
