// Property of Paracosm Industries. Dont use my shit.

#pragma once

#include "CoreMinimal.h"
#include "Components/Inventory/InventoryObject.h"
#include "GameFramework/PlayerState.h"
#include "PlayerStateBase.generated.h"



/**
 * 
 */
UCLASS()
class DIEROBOT_API APlayerStateBase : public APlayerState
{
	GENERATED_BODY()

public:
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Main Inventory")
	UInventoryObject* MainInventory;
	
};
