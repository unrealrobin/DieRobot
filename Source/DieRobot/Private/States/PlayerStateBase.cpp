// Property of Paracosm Industries. Dont use my shit.


#include "States/PlayerStateBase.h"

void APlayerStateBase::BeginPlay()
{
	Super::BeginPlay();

	MainInventory = NewObject<UInventoryObject>(this);
}
