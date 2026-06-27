// Property of Paracosm Industries. Dont use my shit.


#include "UI/Inventory/InventoryWidget.h"

#include "Controller/TimberPlayerController.h"
#include "GameModes/TimberGameModeBase.h"

void UInventoryWidget::NativeConstruct()
{
	Super::NativeConstruct();

	ATimberGameModeBase* GM = Cast<ATimberGameModeBase>(GetWorld()->GetAuthGameMode());
	if (GM)
	{
		if (!GM->OnCharacterInitialization.Contains(this, FName("InitializeInventoryWidget")))
		{
			GM->OnCharacterInitialization.AddDynamic(this, &UInventoryWidget::InitializeInventoryWidget);
		}
		//UE_LOG(LogTemp, Warning, TEXT("Bound to GM's CharacterInitialization Delegate Call."))
	}
	
	
}

void UInventoryWidget::InitializeInventoryWidget()
{
	//UE_LOG(LogTemp, Warning, TEXT("Inventory Widget - Initializing Inventory Widget"));
	ATimberPlayerController* PlayerController = Cast<ATimberPlayerController>(GetOwningPlayer());
	if(IsValid(PlayerController))
	{
		//UE_LOG(LogTemp, Warning, TEXT("Inventory Widget - Player Controller Initialized"));
		PlayerCharacter = Cast<ATimberPlayableCharacter>(PlayerController->GetPawn());
		if(IsValid(PlayerCharacter))
		{
			//UE_LOG(LogTemp, Warning, TEXT("Inventory Widget - Player Character Initialized"));
			PlayerInventory = PlayerCharacter->InventoryObject;
			if(IsValid(PlayerInventory))
			{
				//UE_LOG(LogTemp, Warning, TEXT("Inventory Widget has access to the Players Inventory Object."))
				/* All widget required data is loaded at this point gauranteed based on the Game Mode Initialization Delegate Call, started at Character Begin Play*/
				BindCharacterInventoryData();
			}
		}
	}
}
