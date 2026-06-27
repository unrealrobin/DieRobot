// Property of Paracosm Industries. 


#include "Subsystems/Dialogue/DialogueManager.h"
#include "MetasoundSource.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundMix.h"
#include "Data/DialogueSingle.h"
#include "Data/NarrativeDialogueLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "States/DieRobotGameStateBase.h"

void UDialogueManager::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	
	/*
	if (GetWorld())
	{
		UE_LOG(LogTemp, Warning, TEXT("Dialogue Manager Subsystem Initialized."));
		GetWorld()->GetTimerManager().SetTimerForNextTick(this, &UDialogueManager::LoadNarrativeDialogueLibrary);
		GetWorld()->GetTimerManager().SetTimerForNextTick(this, &UDialogueManager::LoadNarrativeDialogueSoundMix);
	}*/
	LoadNarrativeDialogueLibrary();
	LoadNarrativeDialogueSoundMix();
}

void UDialogueManager::LoadNarrativeDialogueLibrary()
{
	//Get the Dialogue Narrative Library
	static const FString NarrativeDialogueLibraryAssetPath = TEXT("/Game/Sounds/03_Dialogue/DataAssetLibrary/DA_NarrativeDialogueLibrary");
	NarrativeDialogueLibrary = LoadObject<UNarrativeDialogueLibrary>(nullptr, *NarrativeDialogueLibraryAssetPath);
	if (NarrativeDialogueLibrary)
	{
		UE_LOG(LogTemp, Warning, TEXT("Dialogue Manager Subsystem - Loaded the Narrative Dialogue Library."))
	}
}

void UDialogueManager::BindToGameState()
{
	ADieRobotGameStateBase* DieRobotGameState =  Cast<ADieRobotGameStateBase>(GetWorld()->GetGameState());
	if (DieRobotGameState)
	{
		DieRobotGameState->OnTutorialStateChange.AddDynamic(this, &UDialogueManager::HandleTutorialStateChanges);
	}
}

void UDialogueManager::LoadNarrativeDialogueSoundMix()
{
	const FString SoundMixAssetPath = TEXT("/Game/Sounds/02_SoundMix/SCM_DieRobot_NarrativeDialogueMix");

	NarrativeDialogueSoundMix = LoadObject<USoundMix>(nullptr, *SoundMixAssetPath);

	if (NarrativeDialogueSoundMix)
	{
		//UE_LOG(LogTemp, Warning, TEXT("Dialogue Manager Subsystem - Loaded the Narrative Dialogue Sound Mix."));
	}
}

void UDialogueManager::RemoveNarrativeSoundMix()
{
	UGameplayStatics::PopSoundMixModifier(this, NarrativeDialogueSoundMix);
	//UE_LOG(LogTemp, Warning, TEXT("Removed Narrative Dialogue Sound Mix."));
}

UMetaSoundSource* UDialogueManager::GetDialogueVoiceover(FName DialogueName)
{
	if (NarrativeDialogueLibrary && NarrativeDialogueLibrary->NarrativeDialogueList.Num() > 0)
	{
		//UE_LOG(LogTemp, Warning, TEXT("Dialogue Manager - Narrative Dialogue Library and List Exists"));
		UDialogueSingle* DialogueData = *NarrativeDialogueLibrary->NarrativeDialogueList.Find(DialogueName);
		
		if (DialogueData)
		{
			return DialogueData->DialogueVoiceOver;
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Issue Loading the NarrativeDialogueList"));
	}

	UE_LOG(LogTemp, Warning, TEXT("Dialogue Manager Subsystem - GetDialogueVoiceover() -  Dialogue Voiceover Not Found: %s"), *DialogueName.ToString());
	return nullptr;
}

void UDialogueManager::PlayVoiceover(FName VoiceoverName)
{
	/*
	 * Adding the Mix before the Audio Dialogue is played.
	 * Theoretically should decrease the volume of the Music and SFX and Increase the volume of the Dialogue.
	 */
	UGameplayStatics::PushSoundMixModifier(this, NarrativeDialogueSoundMix);
	
	UMetaSoundSource* VoiceoverSound = GetDialogueVoiceover(VoiceoverName);
	
	if (!DialoguePlayer)
	{
		DialoguePlayer = NewObject<UAudioComponent>(this);
		DialoguePlayer->RegisterComponentWithWorld(GetWorld());
		
		DialoguePlayer->OnAudioFinished.AddDynamic(this, &UDialogueManager::RemoveNarrativeSoundMix);
		//UE_LOG(LogTemp, Warning, TEXT("Dialogue Manager - PlayVoiceover - Added Binding to Audio Component to Remove Sound Mix on end Voiceover."));
	}

	if (DialoguePlayer && VoiceoverSound)
	{
		DialoguePlayer->Stop();
		DialoguePlayer->SetSound(VoiceoverSound);
		DialoguePlayer->Play();
		/*
		 * Removing the Mix after the Audio Dialogue is finished playing.
		 * Theoretically should then Increase the Music and SFX Volume back to standard.
		 */
		//UE_LOG(LogTemp, Warning, TEXT("Dialogue Manager Subsystem - Playing Voiceover: %s"), *VoiceoverName.ToString());
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Dialogue Manager Subsystem - PlayVoiceover() -  Voiceover Not Found: %s"), *VoiceoverName.ToString());
	}

	HandlePlayedDialogue(VoiceoverName);
}

void UDialogueManager::HandlePlayedDialogue(FName VoiceoverName)
{
	if (VoiceoverName == FName("Molly_Wake_1"))
	{
		DialoguePlayer->OnAudioFinished.AddDynamic(this, &UDialogueManager::HandleWake1Finish);
	}else if (VoiceoverName == FName("Molly_Wake_2"))
	{
		DialoguePlayer->OnAudioFinished.RemoveDynamic(this, &UDialogueManager::HandleWake1Finish);
	}else if (VoiceoverName == FName("Molly_Wake_3"))
	{
		DialoguePlayer->OnAudioFinished.AddUniqueDynamic(this, &UDialogueManager::HandleWake3Finish);
	}else if (VoiceoverName == FName("Molly_Parts_1"))
	{
		DialoguePlayer->OnAudioFinished.AddDynamic(this, &UDialogueManager::HandleParts1Finish);
	}
	else if (VoiceoverName == FName("Molly_Combat_1"))
	{
		DialoguePlayer->OnAudioFinished.RemoveDynamic(this, &UDialogueManager::HandleWake3Finish);
	}
	else if (VoiceoverName == FName("Molly_WaveStart"))
	{
		DialoguePlayer->OnAudioFinished.AddDynamic(this, &UDialogueManager::HandleWaveStartDialogueFinish);
	}
	else if (VoiceoverName == FName("Molly_WaveComplete"))
	{
		ADieRobotGameStateBase* DieRobotGameStateBase = Cast<ADieRobotGameStateBase>(GetWorld()->GetGameState());
		if (DieRobotGameStateBase)
		{
			DieRobotGameStateBase->ChangeTutorialGameState(ETutorialState::TutorialComplete);
		}
	}
	
}

void UDialogueManager::HandleTutorialStateChanges(ETutorialState NewState)
{
	switch (NewState)
	{
	case ETutorialState::Wake1:
		PlayVoiceover("Molly_Wake_1");
		break;
	case ETutorialState::Wake2:
		PlayVoiceover("Molly_Wake_2");
		break;
	case ETutorialState::Wake3:
		PlayVoiceover("Molly_Wake_3");
		break;
	case ETutorialState::Combat1:
		PlayVoiceover("Molly_Combat_1");
		break;
	case ETutorialState::Combat2:
		//PlayVoiceover("Molly_Combat_Quip_1");
		break;
	case ETutorialState::Parts1:
		PlayVoiceover("Molly_Parts_1");
		break;
	case ETutorialState::Building1:
		PlayVoiceover("Molly_Build_1");
		break;
	case ETutorialState::Building2:
		PlayVoiceover("Molly_WaveStart");
		break;
	case ETutorialState::Building3:
		break;
	case ETutorialState::WaveStart:
		//Play some dialogue here about combat.
			// "You're Rifle is good for long range shots but does less damage, your sword is great for big damage up close."
		break;
	case ETutorialState::WaveComplete:
		PlayVoiceover("Molly_WaveComplete");
		break;
	case ETutorialState::Default:
		break;
	case ETutorialState::TutorialComplete:
		break;
	}
}

/*Tutorial Dialogue Delegates*/
void UDialogueManager::HandleWake1Finish()
{
	ADieRobotGameStateBase* DieRobotGameState = GetWorld()->GetGameState<ADieRobotGameStateBase>();
	if (DieRobotGameState)
	{
		//State Change initiates Broadcast to Listeners
		DieRobotGameState->ChangeTutorialGameState(ETutorialState::Wake2);

		//Playing the Next Voiceover
		//PlayVoiceover(FName("Molly_Wake_2"));
	}
}

void UDialogueManager::HandleWake3Finish()
{
	ADieRobotGameStateBase* DieRobotGameState = GetWorld()->GetGameState<ADieRobotGameStateBase>();
	//We check for Wake3 Current State because if player kills the dummy before the dialogue finishes, we dont want 
	// to change the state backwards, we just continue from the supposed current state, Skipping any unfinished 
	// tutorial dialogue, HUD displays etc.
	if (DieRobotGameState && DieRobotGameState->TutorialState == ETutorialState::Wake3)
	{
		//State Change initiates Broadcast to Listeners
		DieRobotGameState->ChangeTutorialGameState(ETutorialState::Combat1);

		//Initiating Combat Training
		//PlayVoiceover("Molly_Combat_1");

		//Removing the Binding.
		//DialoguePlayer->OnAudioFinished.RemoveDynamic(this, &UDialogueManager::HandleWake3Finish);
	}
}

void UDialogueManager::HandleParts1Finish()
{
	ADieRobotGameStateBase* DieRobotGameState = GetWorld()->GetGameState<ADieRobotGameStateBase>();
	if (DieRobotGameState)
	{
		//State Change initiates Broadcast to Listeners
		DieRobotGameState->ChangeTutorialGameState(ETutorialState::Building1);

		//Removing the Binding.
		DialoguePlayer->OnAudioFinished.RemoveDynamic(this, &UDialogueManager::HandleParts1Finish);
	}
}

void UDialogueManager::HandleWaveStartDialogueFinish()
{
	ADieRobotGameStateBase* DieRobotGameState = GetWorld()->GetGameState<ADieRobotGameStateBase>();
	if (DieRobotGameState)
	{
		//State Change initiates Broadcast to Listeners
		DieRobotGameState->ChangeTutorialGameState(ETutorialState::WaveStart);
		//UE_LOG(LogTemp, Warning, TEXT("Dialogue Manager - HandleWaveStartDialogueFinish - WaveStart State Change."));
		//Removing the Binding.
		DialoguePlayer->OnAudioFinished.RemoveDynamic(this, &UDialogueManager::HandleWaveStartDialogueFinish);
	}
}

