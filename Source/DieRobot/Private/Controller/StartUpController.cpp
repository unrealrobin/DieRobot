// Property of Paracosm Industries. Dont use my shit.


#include "Controller/StartUpController.h"

void AStartUpController::BeginPlay()
{
	Super::BeginPlay();

	bShowMouseCursor = true;
	bEnableClickEvents = true;
	bEnableMouseOverEvents = true;

	FInputModeUIOnly InputMode;
	//Set Widget To Focus
	SetInputMode(InputMode);
	
}
