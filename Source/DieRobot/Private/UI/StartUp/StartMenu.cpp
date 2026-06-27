// Property of Paracosm Industries.


#include "UI/StartUp/StartMenu.h"

#include "Blueprint/WidgetTree.h"
#include "Components/VerticalBox.h"
#include "Subsystems/Online/Login.h"
#include "UI/StartUp/DRLoadMenu.h"

void UStartMenu::NativeConstruct()
{
	Super::NativeConstruct();

	LoginSubsystem = GetGameInstance()->GetSubsystem<ULogin>();
	if (LoginSubsystem)
	{
		LoginSubsystem->IsPlayerLoggedIn.AddDynamic(this, &UStartMenu::HandleUserLogin);
	}
}

void UStartMenu::HandleUserLogin(bool bIsPlayerLoggedIn)
{
	bIsPlayerLoggedInOnline = bIsPlayerLoggedIn;

	if (bIsPlayerLoggedInOnline)
	{
		LoggedInUserDisplayName = LoginSubsystem->LocalUserInfo.OnlineUserDisplayName;
	}
}

void UStartMenu::HideStartMenuSelections()
{
	if (UWidget* FoundWidget = WidgetTree->FindWidget("StartUpMenuVBox"))
	{
		MenuVBox = Cast<UVerticalBox>(FoundWidget);
		if (MenuVBox)
		{
			TArray<UWidget*> ChildrenWidgets = MenuVBox->GetAllChildren();
			for (UWidget* Child : ChildrenWidgets)
			{
				Child->SetVisibility(ESlateVisibility::Collapsed);
			}
		}
	}
}

void UStartMenu::CloseLoadMenu()
{
	if (LoadMenuWidgetRef)
	{
		LoadMenuWidgetRef->OnMainMenuButtonClickedDelegate.RemoveDynamic(this, &UStartMenu::CloseLoadMenu);
		LoadMenuWidgetRef->RemoveFromParent();
		LoadMenuWidgetRef = nullptr;
	}

	ShowStartMenuSelections();
}

void UStartMenu::ShowStartMenuSelections()
{
	if (MenuVBox)
	{
		TArray<UWidget*> ChildrenWidgets = MenuVBox->GetAllChildren();
		for (UWidget* Child : ChildrenWidgets)
		{
			Child->SetVisibility(ESlateVisibility::Visible);
		}
	}
}

void UStartMenu::DisplayLoadMenu()
{
	HideStartMenuSelections();

	if (LoadMenuWidgetClass)
	{
		UUserWidget* Widget = CreateWidget(this, LoadMenuWidgetClass);
		LoadMenuWidgetRef = Cast<UDRLoadMenu>(Widget);
		if (LoadMenuWidgetRef)
		{
			LoadMenuWidgetRef->AddToViewport();
			LoadMenuWidgetRef->SetVisibility(ESlateVisibility::Visible);
			LoadMenuWidgetRef->DisplayAllSavedGames();
			
			LoadMenuWidgetRef->OnMainMenuButtonClickedDelegate.AddDynamic(this, &UStartMenu::CloseLoadMenu);
		}
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("LoadMenuWidgetClass is NULL"));
	}
	
}
