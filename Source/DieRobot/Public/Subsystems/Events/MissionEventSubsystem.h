// Property of Paracosm.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Types/MissionEventPayloads/MissionEventPayload.h"
#include "MissionEventSubsystem.generated.h"

/**
 * 
 */
UCLASS()
class DIEROBOT_API UMissionEventSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnMissionEvent, FMissionEventPayload, Payload);
	FOnMissionEvent OnMissionEvent;

	void BroadcastMissionEvent(FMissionEventPayload Payload);
};
