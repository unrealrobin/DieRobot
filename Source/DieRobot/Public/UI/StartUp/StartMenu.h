// Property of Paracosm Industries. 

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "StartMenu.generated.h"


class UDRLoadMenu;
using namespace UE::Online;
class ULogin;
class UVerticalBox;
/**
 * 
 */
UCLASS()
class DIEROBOT_API UStartMenu : public UUserWidget
{
	GENERATED_BODY()


public:
	virtual void NativeConstruct() override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Load Menu WBP Class")
	TSubclassOf<UDRLoadMenu> LoadMenuWidgetClass;
	
	UPROPERTY()
	ULogin* LoginSubsystem = nullptr;
	
	UFUNCTION(BlueprintCallable)
	void HandleUserLogin(bool bIsPlayerLoggedIn);
	
	UFUNCTION(BlueprintCallable)
	void HideStartMenuSelections();

	UFUNCTION(BlueprintCallable)
	void CloseLoadMenu();

	UFUNCTION(BlueprintCallable)
	void ShowStartMenuSelections();

	UFUNCTION(BlueprintCallable)
	void DisplayLoadMenu();

protected:
	UPROPERTY(BlueprintReadOnly, Category="Start Menu")
	bool bIsPlayerLoggedInOnline = false;

	UPROPERTY(BlueprintReadOnly, Category="Start Menu")
	FString LoggedInUserDisplayName = "";

private:
	UPROPERTY()
	UVerticalBox* MenuVBox = nullptr;

	UPROPERTY()
	UDRLoadMenu* LoadMenuWidgetRef = nullptr;
};
