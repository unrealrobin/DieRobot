// Property of Paracosm Industries.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "InventoryObject.generated.h"

/**
 * 
 */
UCLASS()
class DIEROBOT_API UInventoryObject : public UObject
{
	GENERATED_BODY()

public:
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Inventory Items")
	int NumberOfParts = 100;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Inventory Items")
	int NumberOfMechanism = 100;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Inventory Items")
	int NumberOfUniques = 40;
};

