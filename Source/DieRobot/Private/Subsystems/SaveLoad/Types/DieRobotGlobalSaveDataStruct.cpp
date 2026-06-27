#pragma once
#include "Subsystems/SaveLoad/Types/DieRobotGlobalSaveDataStruct.h"

FString UDieRobotGlobalSaveData::GetCurrentTimeStamp()
{
	FDateTime CurrentDateTime = FDateTime::Now();
	FString DataTimeAsString = CurrentDateTime.ToString();
	return DataTimeAsString;
}
