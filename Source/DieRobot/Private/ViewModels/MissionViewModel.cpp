// Property of Paracosm.


#include "ViewModels/MissionViewModel.h"

void UMissionViewModel::GenerateMissionObjectiveString(UMissionBase* ActiveMission)
{
	if (IsValid(ActiveMission))
	{
		FText ObjectiveText;

		if (ActiveMission->MissionEventType == FGameplayTag::RequestGameplayTag("Event.Combat.Destroy"))
		{
			//TODO::Tentative, Maybe Unnecessary.
			// Now we need to take into consideration whether the other Context Tags Exist. Which we don't like. 
		}
	}
}

FString UMissionViewModel::GetMissionTitle() const
{
	return MissionTitle;
}

void UMissionViewModel::SetMissionTitle(FString NewMissionTitle)
{
	if (UE_MVVM_SET_PROPERTY_VALUE(MissionTitle, NewMissionTitle))
	{
		UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(MissionTitle);
	}
	//MissionTitle = NewMissionTitle;
}
void UMissionViewModel::SetMissionDescription(FString NewMissionDescription)
{
	if (UE_MVVM_SET_PROPERTY_VALUE(MissionDescription, NewMissionDescription))
	{
		UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(MissionDescription);
	}
	//MissionDescription = NewMissionDescription;
}
void UMissionViewModel::SetProgressAmount(int NewProgressAmount)
{
	if (UE_MVVM_SET_PROPERTY_VALUE(ProgressAmount, NewProgressAmount))
	{
		UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(ProgressAmount);
	}
	//ProgressAmount = NewProgressAmount;
}

EMissionState UMissionViewModel::GetActiveMissionState() const
{
	return ActiveMissionState;
}

void UMissionViewModel::SetActiveMissionState(EMissionState NewActiveMissionState)
{
	if (UE_MVVM_SET_PROPERTY_VALUE(ActiveMissionState, NewActiveMissionState))
	{
		UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(ActiveMissionState);
	}
}

FString UMissionViewModel::GetMissionObjectiveString() const
{
	return MissionObjectiveString;
}

void UMissionViewModel::SetMissionObjectiveString(FString NewMissionObjectiveString)
{
	if (UE_MVVM_SET_PROPERTY_VALUE(MissionObjectiveString, NewMissionObjectiveString))
	{
		UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(MissionObjectiveString);
	}
}

int UMissionViewModel::GetGoalValue() const
{
	return GoalValue;
}

void UMissionViewModel::SetGoalValue(int NewGoalValue)
{
	if (UE_MVVM_SET_PROPERTY_VALUE(GoalValue, NewGoalValue))
	{
		UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(GoalValue);
	}
}

void UMissionViewModel::SetProgressPercent(float NewProgressPercent)
{
	if (UE_MVVM_SET_PROPERTY_VALUE(ProgressPercent, NewProgressPercent))
	{
		UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(ProgressPercent);
	}
	//ProgressPercent = NewProgressPercent;
}

FString UMissionViewModel::GetMissionDescription() const
{
	return MissionDescription;
}

float UMissionViewModel::GetProgressPercent() const
{
	return ProgressPercent;
}

int UMissionViewModel::GetProgressAmount() const
{
	return ProgressAmount;
}

bool UMissionViewModel::GetbHasCount() const
{
	return bHasCount;
}

void UMissionViewModel::SetbHasCount(bool NewbHasCount)
{
	if (UE_MVVM_SET_PROPERTY_VALUE(bHasCount, NewbHasCount))
	{
		UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(bHasCount);
	}
}

int UMissionViewModel::GetRewardsParts() const
{
	return RewardsParts;
}

void UMissionViewModel::SetRewardsParts(int NewRewardsParts)
{
	if (UE_MVVM_SET_PROPERTY_VALUE(RewardsParts, NewRewardsParts))
	{
		UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(RewardsParts);
	}
}

int UMissionViewModel::GetRewardsMechanisms() const
{
	return RewardsMechanisms;
}

void UMissionViewModel::SetRewardsMechanisms(int NewRewardsMechanisms)
{
	if (UE_MVVM_SET_PROPERTY_VALUE(RewardsMechanisms, NewRewardsMechanisms))
	{
		UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(RewardsParts);
	}
}

int UMissionViewModel::GetRewardsUniques() const
{
	return RewardsUniques;
}

void UMissionViewModel::SetRewardsUniques(int NewRewardsUniques)
{
	if (UE_MVVM_SET_PROPERTY_VALUE(RewardsUniques, NewRewardsUniques))
	{
		UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(RewardsUniques);
	}
}

ERewardType UMissionViewModel::GetActiveMissionRewardsType() const
{
	return ActiveMissionRewardsType;
}

void UMissionViewModel::SetActiveMissionRewardsType(ERewardType NewActiveMissionRewardsType)
{
	if (UE_MVVM_SET_PROPERTY_VALUE(ActiveMissionRewardsType, NewActiveMissionRewardsType))
	{
		UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(ActiveMissionRewardsType);
	}
}
