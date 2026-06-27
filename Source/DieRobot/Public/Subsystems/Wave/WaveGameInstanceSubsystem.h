// Property of Paracosm Industries.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "WaveGameInstanceSubsystem.generated.h"

class AGarageDoorBase;
class ADieRobotEnemyCharacter;

UENUM(BlueprintType)
enum class EWaveStopReason : uint8
{
	Success,
	Failure,
	LevelSwitch
};

/**
 * 
 */
UCLASS(Blueprintable)
class DIEROBOT_API UWaveGameInstanceSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	void CallForLeaderboardSubsystemBindings();
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	UPROPERTY(BlueprintReadOnly)
	int CurrentWaveNumber = 1;
	
	void SetCurrentWaveNumber(int InWaveNumber);

	/* Delegates */
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FCurrentWaveHandle, int, CurrentWaveNumber);
	FCurrentWaveHandle CurrentWaveHandle;
	DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOpenLabDoorHandle);
	FOpenLabDoorHandle OpenLabDoorHandle;
	DECLARE_DYNAMIC_MULTICAST_DELEGATE(FCloseLabDoorHandle);
	FOpenLabDoorHandle CloseLabDoorHandle;
	DECLARE_DYNAMIC_MULTICAST_DELEGATE(FSaveCurrentGameHandle);
	FSaveCurrentGameHandle SaveCurrentGameHandle;
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FTimeToNextWaveSecondsHandle, int, TimeToNextWaveSeconds);
	FTimeToNextWaveSecondsHandle TimeToNextWaveSecondsHandle;
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FHandleWaveComplete, int, CompletedWaveNumber);
	FHandleWaveComplete OnWaveComplete;
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FHandleWaveStart, int, CompletedWaveNumber);
	FHandleWaveComplete OnWaveStart;
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnBossSpawned, AActor*, BossActor);
	FOnBossSpawned OnBossSpawned;

	/* Data Table */
	//V1
	UFUNCTION()
	void SetWaveCompositionDataTable(UDataTable* DataTable);
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Wave Data")
	UDataTable* WaveCompositionTable;
	//V2
	UFUNCTION()
	void SetWaveCompositionCurveTable(UDataAsset* WaveCompDataAsset);
	UPROPERTY(VisibleAnywhere, Category = "Wave Data")
	UDataAsset* WaveCurveCompositionTable;

	/* Preparing SpawnPoints for Enemies */
	UFUNCTION()
	void PrepareSpawnPoints();
	UFUNCTION()
	void GetAllStandardSpawnPointsInLab();
	UFUNCTION()
	void StageAllStandardSpawnPointLocations();
	UFUNCTION()
	void GetBossSpawnPointLocation();
	UFUNCTION()
	FVector GetRandomStandardSpawnPointLocation();
	UFUNCTION()
	void GetGarageDoor();
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Spawn Points")
	TArray<FVector> StandardSpawnPointLocations;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Spawn Points")
	TArray<AActor*> StandardSpawnPoints;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Spawn Points")
	FVector BossSpawnPointLocation;

	/*Wave Spawning and Composition*/
	UFUNCTION()
	void StartWave();
	UFUNCTION()
	void ComposeWaveFromDataTable();
	UFUNCTION()
	void ComposeWaveFromCurveTable();
	UFUNCTION()
	void HandleBossSpawn();
	UFUNCTION()
	void SpawnBoss(TSubclassOf<AActor> ActorToSpawn, FVector Location);
	UFUNCTION()
	void SpawnPartOfWave();
	UFUNCTION()
	void SpawnWave();
	UFUNCTION()
	void IncrementWave();
	UFUNCTION()
	void SpawnEnemy(TSubclassOf<AActor> ActorToSpawn, FVector Location);
	UFUNCTION()
	void EarlyStartWave();
	UFUNCTION()
	void EndWave(EWaveStopReason WaveStopReason);
	UFUNCTION()
	void FailWave();
	UFUNCTION()
	void SuccessfulWaveEnd();
	
	UPROPERTY()
	TArray<TSubclassOf<ADieRobotEnemyCharacter>> EnemiesToSpawn;
	UFUNCTION()
	void ShuffleEnemiesToSpawn();
	UPROPERTY()
	TSubclassOf<AActor> BossToSpawn = nullptr;
	UPROPERTY()
	int TotalEnemiesToSpawn = 0;
	UPROPERTY()
	int TotalEnemiesSpawned = 0;
	UPROPERTY()
	int TotalEnemiesKilled = 0;
	UPROPERTY()
	bool bAllEnemiesSpawned = false;
	
	FTimerHandle SpawnPartOfWaveTimerHandle;
	
	float TimeBetweenEnemySpawns = 5.f;
	
	/*Enemies*/
	UPROPERTY()
	TArray<ADieRobotEnemyCharacter*> SpawnedEnemies;
	UFUNCTION()
	void CheckArrayForEnemy(ADieRobotEnemyCharacter* Enemy);
	
	UFUNCTION()
	void ResetWaveEnemies();
	UPROPERTY()
	bool BossSpawned = false;
	UFUNCTION()
	void CheckEnemiesForWeapons(ADieRobotEnemyCharacter* Enemy);
	
	/*Wave Iteration*/
	UPROPERTY()
	int TimeBetweenWaves = 120; //2 minutes

	UPROPERTY()
	bool bIsWaveActive = false;

	//Time Remaining to start of next wave.
	UPROPERTY(BlueprintReadOnly)
	int TimeToNextWave = 0;
	FTimerHandle TimeToNextWaveHandle;
	UFUNCTION()
	void UpdateTimeToNextWave();
	FTimerHandle UpdatedTimeToNextWaveTimerHandle;
	

	/*Boss*/
	UPROPERTY()
	AGarageDoorBase* BossDoor;
	UFUNCTION()
	void OpenBossDoor();
	UFUNCTION()
	void CloseBossDoor();
	FTimerHandle BossSpawnDelayTimerHandle;
	UFUNCTION()
	void BindToBossDelegate(ABossBase* BossActor);

	/*Sounds*/
	void PlayWaveStartSound();
	void PlayWaveEndSound();
	void PlayBossSpawnSound();

	UFUNCTION(BlueprintCallable)
	void ResetWaveSubsystem();

private:
	void AddClassToEnemiesToSpawnArray(TSubclassOf<ADieRobotEnemyCharacter> ClassToAdd, int NumberToAdd);
	int GetNumberOfEnemiesToSpawnPerGroup(); //Scales Number of enemies to spawn per "Grouping" by wave Number.
};


