// Property of Paracosm.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Data/DataAssets/MissionSystem/MissionBase.h"
#include "Types/MissionEventPayloads/MissionEventPayload.h"
#include "MissionDeliveryComponent.generated.h"

class UMissionViewModel;
class UMissionBase;

UENUM(BlueprintType) 
enum class EMissionState : uint8
{
	Incomplete UMETA(DisplayName = "Not Complete"),
	InProgress UMETA(DisplayName = "In Progress"),
	Complete UMETA(DisplayName = "Complete"),
	Default UMETA(DisplayName = "Default")
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class DIEROBOT_API UMissionDeliveryComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	
	UMissionDeliveryComponent();

	/*
	 * Stores a List of Missions as a Data Asset.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Missions")
	TObjectPtr<UDataAsset> MissionsListDataAsset = nullptr;

	/*
	 * Stores each mission's individual GUID after completion.
	 *
	 * This array is then saved and repopulated when Loaded.
	 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Missions")
	TArray<FGuid> CompletedMissionGuids;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Missions")
	EMissionState ActiveMissionState = EMissionState::Default;

	/*
	 * Used for Debugging and Logs
	 */
	FString GetMissionTitle();
	
protected:
	
	virtual void BeginPlay() override;

	/*
	 * Active Mission Slot.
	 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Missions")
	TObjectPtr<UMissionBase> ActiveMission = nullptr;

	/*
	 * Mission view Model
	 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Missions")
	TObjectPtr<UMissionViewModel> MissionViewModel = nullptr;
	
private:

	void BindToWaveSubsystem();

	UPROPERTY()
	int CurrentWaveNumber = 0;

	/*
	 * Delegate Callback from MissionEventSubsystem Broadcasts
	 */
	UFUNCTION()
	void ProcessMissionEvent(FMissionEventPayload Payload);

	/*
	 * Sets Uncompleted Missions as Active at the starts of a new Wave.
	 */
	UFUNCTION()
	void HandleWaveStart(int CompletedWaveNumber);

	/*
	 * Validates Mission state once a Wave is completed and marks incomplete if required.
	 */
	UFUNCTION()
	void HandleWaveEnd(int CompletedWaveNumber);

	/*
	 * Binding to the Mission Event Subsystem to listen for Event Broadcasts
	 */
	void BindToMissionEventSystems();

	/*
	 * Sets a Mission as active.
	 */
	void SetActiveMission();

	/*
	 * Populates View Model with new Mission Base Properties. (Ex. Mission title, Description, etc..)
	 */
	void InitializeActiveMission(UMissionBase* NewActiveMission);

	/*
	 * Clears the View Model so old values dont persist on Mission Change.
	 */
	void ResetMissionViewModel();

	/*
	 * Plays Mission Dialogue stored on the Active Mission.
	 */
	void PlayMissionDialogue();

	/*
	 * Updates View Model on Mission State Changes.
	 */
	void UpdateMissionState(EMissionState NewMissionState);

	/*
	 * View Model Getter
	 */
	void GetMissionViewModel();

	/*
	 * Handles Mission State Processes based on Completion Status.
	 */
	void MarkMissionAsCompleted();
	void MarkMissionAsIncomplete();

	/*
	 * Used to create Dynamic Objective Text for Mission Display UI
	 */
	void CreateObjectivesFromMission();
	void FormCombatObjectivesFromTag(FGameplayTag EventTag);
	void FormBuildObjectiveFromTag(FGameplayTag EventTag);

	/*
	 * Calls functions based on Reward Type for Reward Retrieval.
	 */
	void ProcessRewards(TObjectPtr<UMissionBase>& ActiveMissionRef);

	/*
	 * Rewards the Player Currency into their Inventory.
	 */
	void RewardPlayerCurrency(TObjectPtr<UMissionBase>& ActiveMissionRef);

	/*
	 * Compares Payload Context Tags and Event Tags with the Active Mission Required Tags to Validate
	 * the Event.
	 */
	bool CheckMissionContext(FMissionEventPayload& Payload, TWeakObjectPtr<UMissionBase> ActiveMissionRef);
	
};
