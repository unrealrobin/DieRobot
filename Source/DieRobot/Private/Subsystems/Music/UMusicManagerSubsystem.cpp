// Property of Paracosm Industries. Dont use my shit.


#include "Subsystems/Music/UMusicManagerSubsystem.h"

#include "Components/AudioComponent.h"
#include "Data/MusicLibraryDataAsset.h"
#include "Kismet/GameplayStatics.h"


void UUMusicManagerSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	GetWorld()->GetTimerManager().SetTimerForNextTick(this, &UUMusicManagerSubsystem::HandleInitialization);

	BlankTrack = LoadObject<UMetaSoundSource>(nullptr, TEXT("/Game/Sounds/04_Music/MetaSounds/MS_Blank"));
	
}

void UUMusicManagerSubsystem::Deinitialize()
{
	Super::Deinitialize();

	MusicPlayerAlpha = nullptr;
}

//Called from the first world (StartupLevel) so we can use the Create2dSound function and set to Persistant Sound.
void UUMusicManagerSubsystem::HandleInitialization()
{
	//Setting the WorldContext on the Subsystem, when new GameModes Call this function, resets the world context.
	if (!WorldContext || WorldContext != GetWorld())
	{
		WorldContext = GetWorld();
	}
	
	static const FString MusicLibaryAssetPath = TEXT("/Game/Sounds/04_Music/DataAssets/DA_MusicLibrary.DA_MusicLibrary");

	//Getting the Music Library Data Asset
	MusicLibrary = LoadObject<UMusicLibraryDataAsset>(nullptr, *MusicLibaryAssetPath);

	if (!MusicPlayerAlpha)
	{
		//Game/Music/MetaSounds/MS_Blank.MS_Blank'
		
		//MusicPlayer = NewObject<UAudioComponent>(this);
		MusicPlayerAlpha = UGameplayStatics::SpawnSound2D(GetWorld(),
			BlankTrack,
			1.0f,
			1.0f,
			0.0f,
			nullptr,
			true,
			false
			);
		
		if (MusicPlayerAlpha)
		{
			MusicPlayerAlpha->bAutoActivate = true;
			MusicPlayerAlpha->bIsUISound = true;
			//UE_LOG(LogTemp, Warning, TEXT("Audio Object - Music Player - Created"));
		}
		else
		{
			//UE_LOG(LogTemp, Warning, TEXT("Audio Object - Music Player - Not Created"));
		}
	}
}

UAudioComponent* UUMusicManagerSubsystem::GetUnusedAudioComponent()
{
	if (!MusicPlayerAlpha) return MusicPlayerAlpha;
	if (!MusicPlayerBeta) return MusicPlayerBeta;

	return MusicPlayerAlpha->IsPlaying() ? MusicPlayerBeta : MusicPlayerAlpha;
	
}

void UUMusicManagerSubsystem::PlayMusic(FName TrackName, float FadeTime)
{
	//Are the Music Player and Music Library Initialized? (HandleInitialization())
	if (!MusicLibrary || !MusicPlayerAlpha)
	{
		//UE_LOG(LogTemp, Warning, TEXT("Music Player or Music Library not found."));
		return;
	}
	
	//Does track exist in the Music Library?
	if (!MusicLibrary->MusicTrackList.Contains(TrackName))
	{
		//UE_LOG(LogTemp, Warning, TEXT("Track %s not found in Music Library"), *TrackName.ToString());
		return;
	}

	//Get the Tracks Data Asset from the Music Library - @param TrackName
	UMusicTrackDataAsset* TrackData = *MusicLibrary->MusicTrackList.Find(TrackName);
	UMetaSoundSource* NewTrack = TrackData->TrackMetaSound;
	if (TrackData && NewTrack)
	{
		//If the Music Player Audio Component is playing another song -> Crossfade the songs.
		if (MusicPlayerAlpha->IsPlaying())
		{
			CrossfadeMusic(NewTrack, 0.3f);
			return;
		}
		//Stops any currently Playing Track
		MusicPlayerAlpha->Stop();
		//Setting Current track to the new track <MetaSoundSource>
		CurrentTrack = NewTrack;
		//Setting the Sound to the Music Player
		MusicPlayerAlpha->SetSound(CurrentTrack);
		//Fade In Amount for the Song / @param FadeTime / Works as Play()\
		//Setting Master Volume Mulitplies - Higher is louder - Standard is 1.0f
		MusicPlayerAlpha->SetVolumeMultiplier(0.8f);
		MusicPlayerAlpha->FadeIn(FadeTime);
	}
	else
	{
		//UE_LOG(LogTemp, Warning, TEXT("Track %s not found in Music Library"), *TrackName.ToString());
	}
}

void UUMusicManagerSubsystem::CrossfadeMusic(UMetaSoundSource* NewTrack, float FadeTime)
{
	// Checking Initialization of the Music Player component and that a Music Library Exists.
	if (!MusicLibrary || !MusicPlayerAlpha)
	{
		//UE_LOG(LogTemp, Warning, TEXT("Music Player or Music Library not found."));
		return;
	}

	// Fade out over the specified FadeTime
	MusicPlayerAlpha->FadeOut(1.5, 0.3f);

	FTimerHandle TimerHandle;
	GetWorld()->GetTimerManager().SetTimer(TimerHandle, FTimerDelegate::CreateLambda([this, NewTrack, FadeTime]()
	{
		if (MusicPlayerAlpha)
		{
			// Forcing stop of the current track and resetting the state - Just incase some FadeOut() timing is off.
			MusicPlayerAlpha->OnAudioFinished.Clear();
			//MusicPlayerAlpha->Deactivate();
			//UE_LOG(LogTemp, Warning, TEXT("Forcing Stop of MusicPlayerAlpha"));
			//Prepping the new track in the Music Player
			CurrentTrack = NewTrack;
			MusicPlayerAlpha->SetSound(NewTrack);
			MusicPlayerAlpha->SetVolumeMultiplier(0.8f);
			//UE_LOG(LogTemp, Warning, TEXT("New Tracked Prepped for Fade in: %s"), *NewTrack->GetName());

			// Adding a small delay to ensure the MusicPlayer is set up and Ready
			FTimerHandle InnerTimerHandle;
			GetWorld()->GetTimerManager().SetTimer(InnerTimerHandle, FTimerDelegate::CreateLambda([this, NewTrack, FadeTime]()
			{
				//Fade in the New Track
				MusicPlayerAlpha->FadeIn(1.0f);
				//MusicPlayerAlpha->Play();
				//UE_LOG(LogTemp, Warning, TEXT("New Track Faded In: %s"), *CurrentTrack->GetName());
			}), 0.1f, false);
		}
	}), 0.1, false);
}


