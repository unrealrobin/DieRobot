// Property of Paracosm Industries.


#include "Subsystems/SFX/SFXManagerSubsystem.h"

#include "MetasoundSource.h"
#include "Components/AudioComponent.h"
#include "Data/MusicLibraryDataAsset.h"
#include "Data/MusicTrackDataAsset.h"
#include "Sound/SoundMix.h"
#include "Kismet/GameplayStatics.h"


void USFXManagerSubsystem::BindToSFXDelegates()
{
	//TODO:: Wierd thing here, these are delegates from this class. We are eiuther broadcasting these or we need to Listen to delegates from other classes, otherwise
	// other classes should just call the PlaySound function.

	
	/*Bind to all the delgates that Play Sounds - Call PlaySound with Sound*/
	OnLootPickUp.AddDynamic(this, &USFXManagerSubsystem::PlayLootPickUpSound);
	OnWaveStart.AddDynamic(this, &USFXManagerSubsystem::PlayWaveStartSound);
	OnWaveEnd.AddDynamic(this, &USFXManagerSubsystem::PlayWaveEndSound);
	OnBossSpawn.AddDynamic(this, &USFXManagerSubsystem::PlayBossSpawnSound);
	OnBuildablePlacement.AddDynamic(this, &USFXManagerSubsystem::PlayBuildablePlacementSound);
	OnUISuccess.AddDynamic(this, &USFXManagerSubsystem::PlayUISuccessSound);
	OnUIFailure.AddDynamic(this, &USFXManagerSubsystem::PlayUIFailureSound);
}

void USFXManagerSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	//UE_LOG(LogTemp, Warning, TEXT("SFXManager Initialized."));

	BindToSFXDelegates();
	
	//Gets the SFX Library and stores it in memory.
	GetWorld()->GetTimerManager().SetTimerForNextTick(this, &USFXManagerSubsystem::GetSFXLibrary);
	
}

void USFXManagerSubsystem::GetSFXLibrary()
{
	static const FString SFXLibaryAssetPath = TEXT("/Game/Sounds/01_SFX/DataAssets/Library/DA_SFX_Library");
	
	SFXLibrary = LoadObject<UMusicLibraryDataAsset>(nullptr, *SFXLibaryAssetPath);
	if (SFXLibrary)
	{
		//UE_LOG(LogTemp, Warning, TEXT("SFXManager - SFX Library Loaded."));
	}
}

UMetaSoundSource* USFXManagerSubsystem::GetTrackName(FName TrackName)
{
	//Get the Tracks Data Asset from the SFX Library - @param TrackName
	UMusicTrackDataAsset* TrackData = *SFXLibrary->MusicTrackList.Find(TrackName);
	if (!TrackData)
	{
		UE_LOG(LogTemp, Warning, TEXT("SFXManager - Track Data not found."));
	}
	
	UMetaSoundSource* NewTrack = TrackData->TrackMetaSound;

	return NewTrack;
}

void USFXManagerSubsystem::PlaySound(FName TrackName)
{
	UMetaSoundSource* MetaSound = GetTrackName(TrackName);
	if (!MetaSound) return;
	
	if (!GetWorld()) return;
	
	UAudioComponent* AudioComponent = NewObject<UAudioComponent>(this);
	if (AudioComponent)
	{
		AudioComponent->RegisterComponentWithWorld(GetWorld());
		AudioComponent->SetSound(MetaSound);
		
		AudioComponent->OnAudioFinishedNative.AddLambda([this, TrackName](UAudioComponent* AudioComponent)
		{
			//UE_LOG(LogTemp, Warning, TEXT("SFX Manager - Audio Finished Playing: %s. Lambda function Call."), *TrackName.ToString());
			HandleAudioComponentCleanUp(AudioComponent, TrackName);
		});
		
		AudioComponent->Play();
	}
}

void USFXManagerSubsystem::PlaySoundAtLocation(FName TrackName, FVector Location)
{
	UMetaSoundSource* MetaSound = GetTrackName(TrackName);
	if (!MetaSound) return;
	
	if (!GetWorld()) return;
	
	UAudioComponent* AudioComponent = NewObject<UAudioComponent>(this);
	if (AudioComponent)
	{
		AudioComponent->RegisterComponentWithWorld(GetWorld());
		AudioComponent->SetSound(MetaSound);
		AudioComponent->SetWorldLocation(Location);
		
		AudioComponent->OnAudioFinishedNative.AddLambda([this, TrackName](UAudioComponent* AudioComponent)
		{
			//UE_LOG(LogTemp, Warning, TEXT("SFX Manager - Audio Finished Playing: %s. Lambda function Call."), *TrackName.ToString());
			HandleAudioComponentCleanUp(AudioComponent, TrackName);
		});
		
		AudioComponent->Play();
	}
}

void USFXManagerSubsystem::HandleAudioComponentCleanUp(UAudioComponent* AudioComponent, FName TrackName)
{
	//Used to clean up Sub mixes on Specific Track Play Calls.
	HandleTrackSubmixCleanup(TrackName);
	
	AudioComponent->DestroyComponent();
}

void USFXManagerSubsystem::HandleTrackSubmixCleanup(FName TrackName)
{
	//TODO:: Not such an elegant solution - but it works for now.
	if (TrackName == "BossEntry_1" )
	{
		const FString BossSoundMixAssetPath = TEXT("/Game/Sounds/02_SoundMix/SCM_DieRobot_BossSpawnMix");
		USoundMix* BossSoundMix = LoadObject<USoundMix>(nullptr, *BossSoundMixAssetPath);
		UGameplayStatics::PopSoundMixModifier(this, BossSoundMix);
		UE_LOG(LogTemp, Warning, TEXT("SFX Manager - Removed Boss Sound Mix"));
	}
}

void USFXManagerSubsystem::PlayLootPickUpSound()
{
	//Will need a track Name to play the sound
	PlaySound("LootPickUp_1");	
}

void USFXManagerSubsystem::PlayWaveStartSound()
{
	PlaySound("WaveStartAlarm_1");
}

void USFXManagerSubsystem::PlayWaveEndSound()
{
	PlaySound("WaveComplete_1");
	//PlaySound("WaveComplete_1");
}

void USFXManagerSubsystem::PlayBossSpawnSound()
{
	//Enable Boss Spawn Sound Mix.
	//Load Sound Mix
	const FString BossSoundMixAssetPath = TEXT("/Game/Sounds/02_SoundMix/SCM_DieRobot_BossSpawnMix");
	USoundMix* BossSoundMix = LoadObject<USoundMix>(nullptr, *BossSoundMixAssetPath);
	if (BossSoundMix)
	{
		UGameplayStatics::PushSoundMixModifier(this, BossSoundMix);
		//UE_LOG(LogTemp, Warning, TEXT("SFX Manager - Added Boss Sound Mix"));
	}
	
	PlaySound("BossEntry_1");

	//How would we remove the mix after the sound is finished?
	//UGameplayStatics::PopSoundMixModifier(this, BossSoundMix);
}

void USFXManagerSubsystem::PlayBuildablePlacementSound()
{
	PlaySound("BuildablePlacement_1");
}

void USFXManagerSubsystem::PlayUISuccessSound()
{
	PlaySound("UI_Success_1");
}

void USFXManagerSubsystem::PlayUIFailureSound()
{
	PlaySound("UI_Failure_1");
}

