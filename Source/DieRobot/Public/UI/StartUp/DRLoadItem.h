// Property of Paracosm.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "DRLoadItem.generated.h"

class UTextBlock;
/**
 * 
 */
UCLASS()
class DIEROBOT_API UDRLoadItem : public UUserWidget
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "Wave Data", meta = (BindWidget))
	TObjectPtr<UTextBlock> WaveTextBlock;

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "Wave Data", meta = (BindWidget))
	TObjectPtr<UTextBlock> TimeStampBlock;

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "Wave Data", meta = (BindWidget))
	TObjectPtr<UTextBlock> SaveSlotIdBlock;

protected:

	UFUNCTION(BlueprintCallable)
	void HandleClick();
};
