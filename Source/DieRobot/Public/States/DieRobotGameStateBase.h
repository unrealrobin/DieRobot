// Property of Paracosm Industries.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "DieRobotGameStateBase.generated.h"

class ARecastNavMesh;

UENUM(BlueprintType, meta = (ScriptName = "DieRobotTutorialState_ScriptName"))
enum class ETutorialState : uint8
{
	Wake1 UMETA(DisplayName = "Wake1"),
	Wake2 UMETA(DisplayName = "Wake2"),
	Wake3 UMETA(DisplayName = "Wake3"),
	Combat1 UMETA(DisplayName = "Combat1"),
	Combat2 UMETA(DisplayName = "Combat2"),
	Parts1 UMETA(DisplayName = "Parts1"),
	Building1 UMETA(DisplayName = "Building1"),
	Building2 UMETA(DisplayName = "Building2"),
	Building3 UMETA(DisplayName = "Building3"),
	WaveStart UMETA(DisplayName = "WaveStart"),
	WaveComplete UMETA(DisplayName = "WaveComplete"),
	TutorialComplete UMETA(DisplayName = "TutorialCompleted"),
	Default UMETA(DisplayName = "Default")
};

/**
 * 
 */
UCLASS()
class DIEROBOT_API ADieRobotGameStateBase : public AGameStateBase
{
	GENERATED_BODY()

public:
	/*
	 * Bound Classes
	 * TimberGameModeBase
	 * Controller
	 * HUD
	 */
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnTutorialStateChange, ETutorialState, NewState);
	FOnTutorialStateChange OnTutorialStateChange;

	//Tutorial State when Starting Game With Tutorial
	//UPROPERTY(VisibleAnywhere, Category = "State")
	//ETutorialState TutorialState = ETutorialState::Wake1;

	//Tutorial state when starting Game outside of Tutorial
	UPROPERTY(VisibleAnywhere, Category = "State")
	ETutorialState TutorialState = ETutorialState::TutorialComplete;

	void ChangeTutorialGameState(ETutorialState NewState);
	
};
