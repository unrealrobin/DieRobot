// Property of Paracosm.


#include "UI/StartUp/DRLoadItem.h"

#include "Components/TextBlock.h"
#include "GameModes/StartUpGameMode.h"
#include "Kismet/GameplayStatics.h"
#include "Subsystems/SaveLoad/SaveLoadSubsystem.h"

void UDRLoadItem::HandleClick()
{
	USaveLoadSubsystem* SLS = GetGameInstance()->GetSubsystem<USaveLoadSubsystem>();
	if (SLS)
	{
		FString SlotName = SaveSlotIdBlock->GetText().ToString();
		SLS->SetLoadGameSaveSlot(SlotName);

		AStartUpGameMode* GM = Cast<AStartUpGameMode>(UGameplayStatics::GetGameMode(this));
		if (GM)
		{
			GM->SwitchToLabLevel();
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("Could not get Startup Game Mode."));
		}
	}
}
