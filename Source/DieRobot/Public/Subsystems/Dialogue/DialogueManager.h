// Property of Paracosm Industries.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "DialogueManager.generated.h"

enum class ETutorialState : uint8;
class USoundMix;
class UMetaSoundSource;
class UNarrativeDialogueLibrary;
/**
 * 
 */
UCLASS()
class DIEROBOT_API UDialogueManager : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	UPROPERTY()
	UAudioComponent* DialoguePlayer = nullptr;
	
	UPROPERTY()
	UNarrativeDialogueLibrary* NarrativeDialogueLibrary = nullptr;
	
	UFUNCTION()
	void LoadNarrativeDialogueLibrary();
	
	UPROPERTY()
	USoundMix* NarrativeDialogueSoundMix = nullptr;
	
	void LoadNarrativeDialogueSoundMix();

	UFUNCTION()
	void RemoveNarrativeSoundMix();

	UFUNCTION()
	UMetaSoundSource* GetDialogueVoiceover(FName DialogueName);

	UFUNCTION(BlueprintCallable)
	void PlayVoiceover(FName VoiceoverName);

	UFUNCTION()
	void HandlePlayedDialogue(FName VoiceoverName);
	UFUNCTION()
	void HandleTutorialStateChanges(ETutorialState NewState);

	/*Tutorial Dialogue Handling*/
	UFUNCTION()
	void HandleWake1Finish();
	UFUNCTION()
	void HandleParts1Finish();
	UFUNCTION()
	void HandleWake3Finish();
	UFUNCTION()
	void HandleWaveStartDialogueFinish();

	UFUNCTION()
	void BindToGameState();

};
