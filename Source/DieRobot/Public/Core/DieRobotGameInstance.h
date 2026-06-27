// Property of Paracosm.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "DieRobotGameInstance.generated.h"

/**
 * 
 */
UCLASS()
class DIEROBOT_API UDieRobotGameInstance : public UGameInstance
{
	GENERATED_BODY()

public:

	virtual void Init() override;
	
};
