// Property of Paracosm.


#include "Subsystems/Events/MissionEventSubsystem.h"

void UMissionEventSubsystem::BroadcastMissionEvent(FMissionEventPayload Payload)
{
	OnMissionEvent.Broadcast(Payload);
	
	UE_LOG(LogTemp, Warning, TEXT("Combat Event Broadcasted."));
}
