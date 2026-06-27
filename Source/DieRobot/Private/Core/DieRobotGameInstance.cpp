// Property of Paracosm.


#include "Core/DieRobotGameInstance.h"

#include "GameFramework/GameUserSettings.h"
#include "Subsystems/SaveLoad/SaveLoadSubsystem.h"

void UDieRobotGameInstance::Init()
{
	Super::Init();

	if (GEngine)
	{
		//Returns the Default Game User Settings - DieRobotGameUserSettings
		if (UGameUserSettings* Settings = GEngine->GetGameUserSettings())
		{
			Settings->ApplySettings(false);  
		}
	}
}
