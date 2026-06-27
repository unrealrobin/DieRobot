// Property of Paracosm Industries. Dont use my shit.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "UI/StartUp/StartMenu.h"
#include "StartUpGameMode.generated.h"

/**
 * 
 */
UCLASS()
class DIEROBOT_API AStartUpGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable)
	void SwitchToLabLevel();

protected:
	void InitializeMusicManager();
	void InitializeStartUpMenu();
	void InitializeGameConfigSubsystem();
	void ResetWaveSubsystem();
	virtual void BeginPlay() override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
	TSubclassOf<UUserWidget> StartUpMenuClass;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "UI")
	UUserWidget* StartUpMenu;

	UFUNCTION()
	void SetGameConfig(EDieRobotGameConfigType InGameState);

	UFUNCTION(BlueprintCallable)
	void SwitchToMidgameDemo();

	void InitializeLastLoadedGameSlot();
	
};
