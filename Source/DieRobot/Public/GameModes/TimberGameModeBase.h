// Property of Paracosm Industries. Dont use my shit.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "TimerManager.h"
#include "Subsystems/SaveLoad/SaveLoadStruct.h"
#include "States/DieRobotGameStateBase.h"
#include "TimberGameModeBase.generated.h"

class ALocationMarkerBase;
enum class ETutorialState : uint8;
enum class EDieRobotGameConfigType : uint8;
class ATimberSeeda;
class UWaveGameInstanceSubsystem;
class ATimberPlayableCharacter;
class ALabDoorBase;
class ATimberEnemyCharacter;
class ATimberEnemySpawnLocations;



/**
 * 
 */
UCLASS()
class DIEROBOT_API ATimberGameModeBase : public AGameModeBase
{
	GENERATED_BODY()

public:
	/* Delegate Signature*/
	DECLARE_DYNAMIC_DELEGATE(FSwitchToStandardUI);
	DECLARE_DYNAMIC_DELEGATE(FEnableStandardInputMappingContext);
	DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnCharacterInitialization);
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSeedaSpawn, AActor*, Seeda);

	/* Delegate Handle */
	FSwitchToStandardUI SwitchToStandardUI;
	FEnableStandardInputMappingContext EnableStandardInputMappingContext;
	FOnCharacterInitialization OnCharacterInitialization;
	FOnSeedaSpawn OnSeedaSpawn;
	
	virtual void BeginPlay() override;
	
	void PassDataTableToWaveSubsystem(UDataTable* DataTable);
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Wave Data")
	UDataTable* WaveCompositionDataTable;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Wave Data")
	UDataAsset* WaveCompositionCurveTable;
	
	void PlayerIsInitialized(AActor* InitializedPlayer);
	
	void PathTracer_RedrawDelegateBinding();\
	
	void GatherSeedaData();
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Seeda")
	ATimberSeeda* Seeda = nullptr;

	/* Game State / Tutorial */
	void InitializeGameState();

	UFUNCTION()
	void HandleTutorialStateChange(ETutorialState NewState);
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Game State")
	ETutorialState TutorialState = ETutorialState::Default;
	
	void GetTutorialState();
	
	UFUNCTION()
	void UpdateTutorialState(ETutorialState NewState);

	UFUNCTION()
	void SpawnDummyForTutorial();

	/*Character*/
	UPROPERTY()
	ATimberPlayableCharacter* TimberCharacter = nullptr;
	
	/* Seeda Details */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Classes")
	TSubclassOf<ATimberSeeda> SeedaClass;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Seeda Info")
	FVector SeedaLocation;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TArray<AActor*> ArrayOfSpawnedSeedas;
	
	UFUNCTION()
	void HandleWaveComplete(int CompletedWave);
	
	void OpenAllLabDoors();

	/*Character Freeze*/
	UFUNCTION()
	void FreezeAllAICharacters(bool bIsPlayerDead);

	/* Location Marker */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Spawn")
	TSubclassOf<ALocationMarkerBase> LocationMarker;

	UFUNCTION(BlueprintCallable)
	void SwitchToMainMenu();

	/*Handle Doors */
	UFUNCTION()
	void OpenLabDoors();
	void CloseAllLabDoors();
	UFUNCTION()
	void CloseLabDoors();
	UFUNCTION(BlueprintCallable)
	void GatherAllLabDoors();
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lab Doors")
	TArray<AActor*> ArrayOfLabDoors;

	/* Path Tracing */
	UFUNCTION(BlueprintImplementableEvent)
	void RedrawPathTrace();
	UFUNCTION()
	void HandleRedrawPathTrace();

protected:

	UWaveGameInstanceSubsystem* GetWaveGameInstanceSubsystem();

	UFUNCTION()
	void PlayBuildMusic();
	UFUNCTION()
	void PlayAttackMusic();

private:

	//CI/CD
	void TestSentry();

	void InitializeSaveLoadSession();
	
};
