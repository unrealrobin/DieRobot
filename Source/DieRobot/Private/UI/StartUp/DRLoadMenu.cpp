// Property of Paracosm.


#include "UI/StartUp/DRLoadMenu.h"

#include "Components/TextBlock.h"
#include "Components/ScrollBox.h"
#include "Subsystems/SaveLoad/SaveLoadSubsystem.h"
#include "Subsystems/SaveLoad/Types/DieRobotGlobalSaveDataStruct.h"
#include "UI/StartUp/DRLoadItem.h"
#include "UI/StartUp/StartMenu.h"


void UDRLoadMenu::NativeConstruct()
{
	Super::NativeConstruct();

	GetAllSavedGames();
}

void UDRLoadMenu::HandleMainMenuButtonClicked()
{
	OnMainMenuButtonClickedDelegate.Broadcast();
}

void UDRLoadMenu::GetAllSavedGames()
{
	AllSavedGames.Empty();
	
	USaveLoadSubsystem* SLS = GetGameInstance()->GetSubsystem<USaveLoadSubsystem>();
	if (SLS)
	{
		UDieRobotGlobalSaveData* GSDInstance = SLS->GetGlobalSaveDataInstance();
		if (IsValid(GSDInstance))
		{
			AllSavedGames = GSDInstance->ActiveSaveSlots;
		}
		else
		{
			//UE_LOG(LogTemp, Error, TEXT("LoadMenu - GetAllSavedGames() - Failed to Retrieve Global Save Data Instance."));
		}

		//UE_LOG(LogTemp, Warning, TEXT("Retrieving %d saved games from Global Save Data Instance."), AllSavedGames.Num());
	}
}


void UDRLoadMenu::DisplayAllSavedGames()
{
	/* Loads Global Save Data Slot and Updated the AllSavedGames Variable*/
	GetAllSavedGames();
	
	if (AllSavedGames.Num() > 0)
	{
		//UE_LOG(LogTemp, Warning, TEXT("Beginning Display of Global Save Slots."));
		for (FSaveSlotDataStruct SaveSlot: AllSavedGames)
		{
			//UE_LOG(LogTemp, Warning, TEXT("Save Slot: %ls"), *SaveSlot.SlotName);
			//UE_LOG(LogTemp, Warning, TEXT("CurrentWave: %i"), SaveSlot.SlotCurrentWave);
			//Create Load Menu Item Widgets
			UUserWidget* Widget = CreateWidget(this, LoadItemWidgetClass);
			UDRLoadItem* LoadItemWidget = Cast<UDRLoadItem>(Widget);
			if (VerticalLoadItemBlock && LoadItemWidget)
			{
				LoadItemWidget->WaveTextBlock->SetText(FText::FromString(FString::FromInt(SaveSlot.SlotCurrentWave)));
				LoadItemWidget->TimeStampBlock->SetText(FText::FromString(SaveSlot.LastTimeStamp));
				LoadItemWidget->SaveSlotIdBlock->SetText(FText::FromString(SaveSlot.SlotName));
				
				VerticalLoadItemBlock->AddChild(LoadItemWidget);
				//UE_LOG(LogTemp, Warning, TEXT("Save Slot Complete."));
			}
		}
	}
}
