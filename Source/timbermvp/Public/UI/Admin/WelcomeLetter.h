// Property of Paracosm.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Controller/TimberPlayerController.h"
#include "WelcomeLetter.generated.h"

class UButton;
/**
 * 
 */
UCLASS()
class TIMBERMVP_API UWelcomeLetter : public UUserWidget
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Player")
	TObjectPtr<ATimberPlayerController> DrPlayerController;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Widgets", meta = (BindWidget))
	TObjectPtr<UButton> CloseButton;
};
