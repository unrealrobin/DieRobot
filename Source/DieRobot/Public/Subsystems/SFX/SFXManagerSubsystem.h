// Property of Paracosm Industries. Dont use my shit.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "SFXManagerSubsystem.generated.h"

class UMusicLibraryDataAsset;
class UMetaSoundSource;
/**
 * 
 */
UCLASS()
class DIEROBOT_API USFXManagerSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:

	DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnLootPickUp);
	DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnWaveStart);
	DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnWaveEnd);
	DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnBossSpawn);
	DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnBuildablePlacement);
	DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnUISuccess);
	DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnUIFailure);
	
	FOnLootPickUp OnLootPickUp;
	FOnWaveStart OnWaveStart;
	FOnWaveEnd OnWaveEnd;
	FOnBossSpawn OnBossSpawn;
	FOnBuildablePlacement OnBuildablePlacement;
	FOnUISuccess OnUISuccess;
	FOnUIFailure OnUIFailure;

	void BindToSFXDelegates();
	
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	
	//Creates the Audio Component and Plays the Sound - Called in each function
	void PlaySound(FName TrackName);

	void PlaySoundAtLocation(FName TrackName, FVector Location);
	
protected:
	//Holds all the SFX We need
	UPROPERTY()
	UMusicLibraryDataAsset* SFXLibrary;

	//Getting the Library 1 tick after initialization - just like Music System
	void GetSFXLibrary();

	//this - Will get a track based on the delegate broadcast - events don't declare which sound to play, this subsystem does.
	UMetaSoundSource* GetTrackName(FName TrackName);
	
	UFUNCTION()
	void HandleAudioComponentCleanUp(UAudioComponent* AudioComponent, FName TrackName);
	UFUNCTION()
	void HandleTrackSubmixCleanup(FName TrackName);

	UFUNCTION()
	void PlayLootPickUpSound();
	UFUNCTION()
	void PlayWaveStartSound();
	UFUNCTION()
	void PlayWaveEndSound();
	UFUNCTION()
	void PlayBossSpawnSound();
	UFUNCTION()
	void PlayBuildablePlacementSound();
	UFUNCTION()
	void PlayUISuccessSound();
	UFUNCTION()
	void PlayUIFailureSound();

	//void CleanUpAudioComponent(UAudioComponent* AudioComponent);
};
