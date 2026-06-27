// Property of Paracosm.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "DRLoadMenu.generated.h"


class UScrollBox;
class UDRLoadItem;
class UButton;
struct FSaveSlotDataStruct;
/**
 * 
 */
UCLASS()
class DIEROBOT_API UDRLoadMenu : public UUserWidget
{
	GENERATED_BODY()

public:

	DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnMainMenuButtonClicked);
	FOnMainMenuButtonClicked OnMainMenuButtonClickedDelegate;
	
	virtual void NativeConstruct() override;
	
	void DisplayAllSavedGames();

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Widget Data")
	TSubclassOf<UDRLoadItem> LoadItemWidgetClass;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UScrollBox> VerticalLoadItemBlock;

protected:

	UFUNCTION(BlueprintCallable)
	void HandleMainMenuButtonClicked();
	
private:
	UPROPERTY()
	TArray<FSaveSlotDataStruct> AllSavedGames;

	void GetAllSavedGames();

};
