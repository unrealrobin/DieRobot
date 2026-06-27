// Property of Paracosm.

#pragma once

#include "CoreMinimal.h"
#include "MVVMViewModelBase.h"
#include "Components/MissionDelivery/MissionDeliveryComponent.h"
#include "Data/DataAssets/MissionSystem/MissionBase.h"
#include "MissionViewModel.generated.h"

/**
 * 
 */
UCLASS()
class DIEROBOT_API UMissionViewModel : public UMVVMViewModelBase
{
	GENERATED_BODY()

public:
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	FGuid ActiveMissionGuid;

	void GenerateMissionObjectiveString(UMissionBase* ActiveMission);
	
	FString GetMissionTitle() const;
	void SetMissionTitle(FString NewMissionTitle);
	
	FString GetMissionDescription() const;
	void SetMissionDescription(FString NewMissionDescription);
	
	float GetProgressPercent() const;
	void SetProgressPercent(float NewProgressPercent);
	
	int GetProgressAmount() const;
	void SetProgressAmount(int NewProgressAmount);

	EMissionState GetActiveMissionState() const;
	void SetActiveMissionState(EMissionState NewActiveMissionState);

	FString GetMissionObjectiveString() const;
	void SetMissionObjectiveString(FString NewMissionObjectiveString);
	
	int GetGoalValue() const;
	void SetGoalValue(int NewGoalValue);

	bool GetbHasCount() const;
	void SetbHasCount(bool NewbHasCount);

	int GetRewardsParts() const;
	void SetRewardsParts(int NewRewardsParts);
	
	int GetRewardsMechanisms() const;
	void SetRewardsMechanisms(int NewRewardsMechanisms);

	int GetRewardsUniques() const;
	void SetRewardsUniques(int NewRewardsUniques);

	ERewardType GetActiveMissionRewardsType() const;
	void SetActiveMissionRewardsType(ERewardType NewActiveMissionRewardsType);
	
protected:

	UPROPERTY(BlueprintReadOnly, FieldNotify, Getter)
	EMissionState ActiveMissionState = EMissionState::Default;
	
	UPROPERTY(BlueprintReadOnly, FieldNotify, Getter)
	ERewardType ActiveMissionRewardsType = ERewardType::Default;
	
	UPROPERTY(BlueprintReadOnly, FieldNotify, Getter)
	FString MissionTitle = "Test Mission Title";

	UPROPERTY(BlueprintReadOnly, FieldNotify, Getter)
	FString MissionDescription = "Mission Description Goes Here.";
	
	UPROPERTY(BlueprintReadOnly, FieldNotify, Getter)
	float ProgressPercent = 0.f;

	UPROPERTY(BlueprintReadOnly, FieldNotify, Getter)
	int ProgressAmount = 0;

	UPROPERTY(BlueprintReadOnly, FieldNotify, Getter)
	int GoalValue = 0;
	
	UPROPERTY(BlueprintReadOnly, FieldNotify, Getter)
	int RewardsParts;
	
	UPROPERTY(BlueprintReadOnly, FieldNotify, Getter)
	int RewardsMechanisms;

	UPROPERTY(BlueprintReadOnly, FieldNotify, Getter)
	int RewardsUniques;
	
	UPROPERTY(BlueprintReadOnly, FieldNotify, Getter)
	FString MissionObjectiveString = "Mission Objective";
	
	UPROPERTY(BlueprintReadOnly, FieldNotify, Getter)
	bool bHasCount = false;

	//TODO:: Have an Objective String. With Dynamic Values Highlighted.
	// EX. Destroy [10] [Grunts] with [Hammer] (0/10)
	// EX. Destroy {Count} {Enemy.Carbonite.Grunts} with {Weapon.Hammer} 
	// Content in [content] to be highlightable.
	// Multiple Objectives Should Stack.
	
};



