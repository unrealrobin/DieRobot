// Property of Paracosm.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameUserSettings.h"
#include "DieRobotGameUserSettings.generated.h"

/**
 * 
 */
UCLASS()
class DIEROBOT_API UDieRobotGameUserSettings : public UGameUserSettings
{
	GENERATED_BODY()

public:
	
	virtual void ApplySettings(bool bCheckForCommandLineOverrides) override;
};
