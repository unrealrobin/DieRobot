// Property of Paracosm Industries. Dont use my shit.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/Inventory/InventoryObject.h"
#include "InventoryWidget.generated.h"

class ATimberPlayableCharacter;
/**
 * 
 */
UCLASS()
class DIEROBOT_API UInventoryWidget : public UUserWidget
{
	GENERATED_BODY()

public:

	virtual void NativeConstruct() override;

	UFUNCTION()
	void InitializeInventoryWidget();

	UFUNCTION(BlueprintImplementableEvent)
	void BindCharacterInventoryData();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Player Inventory")
	UInventoryObject* PlayerInventory = nullptr;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Player Inventory")
	ATimberPlayableCharacter* PlayerCharacter;
};
