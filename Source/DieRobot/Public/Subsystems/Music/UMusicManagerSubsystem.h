// Property of Paracosm Industries. 

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "UMusicManagerSubsystem.generated.h"

class UMetaSoundSource;
class UMusicLibraryDataAsset;
/**
 * 
 */
UCLASS()
class DIEROBOT_API UUMusicManagerSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	UPROPERTY()
	UMusicLibraryDataAsset* MusicLibrary;

	UPROPERTY()
	UAudioComponent* MusicPlayerAlpha;

	UPROPERTY()
	UAudioComponent* MusicPlayerBeta;
	
	UPROPERTY()
	UMetaSoundSource* CurrentTrack;

	UFUNCTION()
	void PlayMusic(FName TrackName, float FadeTime=2.0f);

	UFUNCTION()
	void HandleInitialization();

	UFUNCTION()
	UAudioComponent* GetUnusedAudioComponent();
protected:

	UPROPERTY()
	UWorld* WorldContext;

	UPROPERTY()
	UMetaSoundSource* BlankTrack;
	
	void Initialize(FSubsystemCollectionBase& Collection) override;

	void Deinitialize() override;
	
	UFUNCTION()
	void CrossfadeMusic(UMetaSoundSource* NewTrack, float FadeTime=2.0f);
};
