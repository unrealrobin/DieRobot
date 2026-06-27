// Property of Paracosm Industries.


#include "UI/DieRobotHUDBase.h"
#include "Blueprint/UserWidget.h"
#include "Blueprint/WidgetTree.h"
#include "Character/DieRobotSeeda.h"
#include "Character/Enemies/Boss/BossBase.h"
#include "Components/Button.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameModes/DieRobotGameModeBase.h"
#include "Subsystems/Wave/WaveGameInstanceSubsystem.h"
#include "UI/BossHealthBar.h"
#include "UI/BuildingComponentPanel.h"
#include "UI/Admin/WelcomeLetter.h"
#include "UI/Death/DieRobotDeathWidget.h"


void ADieRobotHUDBase::BindToWaveSubsystem()
{
	UWaveGameInstanceSubsystem* WaveSubsystem = GetGameInstance()->GetSubsystem<UWaveGameInstanceSubsystem>();
	if (WaveSubsystem)
	{
		//Broadcast from Wave Subsystem when Boss is spawned, passes in a ref to the spawned boss.
		WaveSubsystem->OnBossSpawned.AddDynamic(this, &ADieRobotHUDBase::HandleBossSpawned);
	}
}

void ADieRobotHUDBase::BeginPlay()
{
	Super::BeginPlay();
	//Setting Controller Owner
	DieRobotPlayerController = Cast<ADieRobotPlayerController>(GetOwningPlayerController());
	if (!DieRobotPlayerController) return;
	InitializeWidgets();
	CharacterAndControllerBindings();
	GameModeBindings();
	SeedaBindings();
	//CreateWelcomeMessageWidget();
		
	//Binding to Tutorial States
	InitializeTutorialStateBinding();
	HandleTutorialStateChanges(GetTutorialState());
	BindToWaveSubsystem();	
	
}

void ADieRobotHUDBase::CreateWelcomeMessageWidget()
{
	WelcomeWidget = CreateVisibleWidget(WelcomeWidgetClass, 100);
	if (WelcomeWidget)
	{
		if (DieRobotPlayerController)
		{
			DieRobotPlayerController->EnableCursor();
			UWelcomeLetter* WelcomeLetter = Cast<UWelcomeLetter>(WelcomeWidget);
			WelcomeLetter->DrPlayerController = DieRobotPlayerController;
			if (WelcomeLetter && WelcomeLetter->CloseButton)
			{
				WelcomeLetter->CloseButton->SetFocus();
			}
			else
			{
				WelcomeLetter->SetFocus();
			}
		}
	}
}

void ADieRobotHUDBase::InitializeWidgets()
{
	/*Initialized Visible*/
	RootWidget = CreateVisibleWidget(RootWidgetClass, 1);

	/*Initialized Hidden*/
	BuildMenuWidget = CreateHiddenWidget(BuildMenuWidgetClass, 10);
	//AmmoCounterWidget = CreateHiddenWidget(AmmoCounterWidgetClass, 1);
	KBM_MovementControlsWidget = CreateHiddenWidget(KBM_MovementControlsWidgetClass, 2);
	KBM_CombatControlsWidget = CreateHiddenWidget(KBM_CombatControlsWidgetClass, 2);
	KBM_BuildControlsWidget = CreateHiddenWidget(KBM_BuildControlWidgetClass, 2);
	DeathWidget = CreateHiddenWidget(DeathWidgetClass, 100);
	BossHealthBarWidget = CreateHiddenWidget(BossHealthBarWidgetClass, 2);
	SettingsPanelWidget = CreateHiddenWidget(SettingsPanelWidgetClass, 10);
	
	
	
}

UUserWidget* ADieRobotHUDBase::CreateHiddenWidget(TSubclassOf<UUserWidget> WidgetClass, int32 ZOrder)
{
	if (WidgetClass)
	{
		UUserWidget* HiddenWidget = CreateWidget<UUserWidget>(GetWorld(), WidgetClass);
		if (HiddenWidget)
		{
			HiddenWidget->AddToViewport(ZOrder);
			HiddenWidget->SetVisibility(ESlateVisibility::Hidden);
			return HiddenWidget;
		}
	}
	//UE_LOG(LogTemp, Error, TEXT("Failed to create widget: %s"), *WidgetClass->GetName());
	return nullptr;
}

void ADieRobotHUDBase::CharacterAndControllerBindings()
{
	
	if (DieRobotPlayerController)
	{
		//UE_LOG(LogTemp, Warning, TEXT("HUD has Cached DieRobot Character Controller"));

		DieRobotPlayerController->IsBuildPanelOpen.AddDynamic(this, &ADieRobotHUDBase::HandleBuildPanelMenu);
		DieRobotPlayerController->ShouldHideBuildMenu.AddDynamic(this, &ADieRobotHUDBase::CloseBuildPanelMenu);
		DieRobotPlayerController->HandleDeathUI_DelegateHandle.BindUFunction(this, FName("SwitchToDeathUI"));
		DieRobotPlayerController->ToggleSettingsPanel_DelegateHandle.AddDynamic(this, &ADieRobotHUDBase::ToggleSettingsPanelWidget);
		//DieRobotPlayerController->ShowAmmoCounter.AddDynamic(this, &ADieRobotHUDBase::HandleAmmoCounterVisibility);

		DieRobotCharacter = Cast<ADieRobotPlayableCharacter>(
			DieRobotPlayerController->GetCharacter());

		if (DieRobotCharacter)
		{
			DieRobotCharacter->HandleSpawnDeleteIconLocation_DelegateHandle.AddDynamic(this, &ADieRobotHUDBase::ShowDeleteBuildingComponentWidget);
			DieRobotCharacter->HandleRemoveDeleteIcon_DelegateHandle.AddDynamic(this, &ADieRobotHUDBase::HideDeleteBuildingComponentWidget);
			DieRobotCharacter->HandlePlayerDeath_DelegateHandle.AddDynamic(this, &ADieRobotHUDBase::UpdateDeathUIReason_KipDestroyed);
		}
	}
}

void ADieRobotHUDBase::GameModeBindings()
{
	ADieRobotGameModeBase* GameMode = Cast<ADieRobotGameModeBase>(GetWorld()->GetAuthGameMode());
	if (GameMode)
	{
		GameMode->SwitchToStandardUI.BindUFunction(this, FName("SwitchToGameUI"));
	}
}

void ADieRobotHUDBase::SeedaBindings()
{
	ADieRobotGameModeBase* GameMode = Cast<ADieRobotGameModeBase>(GetWorld()->GetAuthGameMode());
	if (GameMode)
	{
		ADieRobotSeeda* Seeda = GameMode->Seeda;
		if (Seeda)
		{
			Seeda->OnSeedaDeathUI.AddDynamic(this, &ADieRobotHUDBase::UpdateDeathUIReason_SeedaDestroyed);
			//UE_LOG(LogTemp, Warning, TEXT("Successfully Bound to Seeda Death UI Reason Delegate."))
		}
	}
}

void ADieRobotHUDBase::HandleBossDeath()
{
	//Make Boss Health Bar Hidden
	if (BossHealthBarWidget)
	{
		if (Cast<UBossHealthBar>(BossHealthBarWidget))
		{
			//When the widget is Hidden, it still ticks. So we clear the BossActor ref to prevent all tick functionality that handles Health Updates.
			Cast<UBossHealthBar>(BossHealthBarWidget)->BossActor = nullptr;
		}
		HideWidget(BossHealthBarWidget);
	}
}

void ADieRobotHUDBase::UpdateDeathUIReason_KipDestroyed(bool bIsPlayerDead)
{
	//UE_LOG(LogTemp, Warning, TEXT("Death Delegate Received to Hud from Kip."));
	if (bIsPlayerDead)
	{
		if (BossHealthBarWidget && BossHealthBarWidget->IsVisible())
		{
			HideWidget(BossHealthBarWidget);
		}
		
		if (DeathWidget)
		{
			UDieRobotDeathWidget* DieRobotDeathWidget = Cast<UDieRobotDeathWidget>(DeathWidget);
			if (DieRobotDeathWidget && DieRobotDeathWidget->DeathReason == EDeathReason::Default)
			{
				//UE_LOG(LogTemp, Warning, TEXT("DeathReason is Default, Changing to Kip."));
				DieRobotDeathWidget->DeathReason = EDeathReason::KipDestroyed;
				DieRobotDeathWidget->UpdateDeathReasonText(EDeathReason::KipDestroyed);
			}
		}
	}
}

void ADieRobotHUDBase::ToggleSettingsPanelWidget()
{
	if (SettingsPanelWidget)
	{
		//Toggle based on visibility.
		SettingsPanelWidget->IsVisible() ? HideWidget(SettingsPanelWidget) : ShowWidget(SettingsPanelWidget);

		if (SettingsPanelWidget->IsVisible())
		{
			FInputModeGameAndUI GameAndUIInputMode;
			DieRobotPlayerController->SetInputMode(GameAndUIInputMode);
			DieRobotPlayerController->SetFocusedUserWidget(SettingsPanelWidget);
			DieRobotPlayerController->SetMouseLocation(GetCenterOfScreen().X, GetCenterOfScreen().Y);
			DieRobotPlayerController->EnableCursor();
		}
		else
		{
			FInputModeGameOnly GameOnlyInputMode;
			DieRobotPlayerController->SetInputMode(GameOnlyInputMode);
			DieRobotPlayerController->DisableCursor();
		}
	}
}

void ADieRobotHUDBase::UpdateDeathUIReason_SeedaDestroyed(bool bIsSeedaDestroyed)
{
	//UE_LOG(LogTemp, Warning, TEXT("Death Delegate Received to Hud from Seeda."));
	if (DeathWidget)
	{
		//UE_LOG(LogTemp, Warning, TEXT("Seeda Death Widget Valid."));
		UDieRobotDeathWidget* DieRobotDeathWidget = Cast<UDieRobotDeathWidget>(DeathWidget);
		if (DieRobotDeathWidget)
		{
			//UE_LOG(LogTemp, Warning, TEXT("Seeda - Successful cast to Death Widget Class"));
			if (DieRobotDeathWidget->DeathReason == EDeathReason::Default)
			{
				//UE_LOG(LogTemp, Warning, TEXT("DeathReason is Default, Changing to Seeda."));
				DieRobotDeathWidget->DeathReason = EDeathReason::SeedaDestroyed;
				DieRobotDeathWidget->UpdateDeathReasonText(EDeathReason::SeedaDestroyed);
			}
		}
	}
}

// Called by Player using the "B" Key, Listening for the Delegate on the Controller
void ADieRobotHUDBase::HandleBuildPanelMenu(bool IsBuildPanelMenuOpen)
{
	if (BuildMenuWidget->IsVisible())
	{
		return;
	}

	OpenBuildPanelMenu();
}

void ADieRobotHUDBase::InitializeTutorialStateBinding()
{
	ADieRobotGameStateBase* DieRobotGameState = Cast<ADieRobotGameStateBase>(GetWorld()->GetGameState());
	if (DieRobotGameState)
	{
		DieRobotGameState->OnTutorialStateChange.AddDynamic(this, &ADieRobotHUDBase::HandleTutorialStateChanges);
	}
}

void ADieRobotHUDBase::HideWidget(UUserWidget* Widget)
{
	if (Widget)
	{
		Widget->SetVisibility(ESlateVisibility::Hidden);
	}
}

void ADieRobotHUDBase::ShowWidget(UUserWidget* Widget)
{
	if (Widget)
	{
		Widget->SetVisibility(ESlateVisibility::Visible);
	}
}

void ADieRobotHUDBase::SetWidgetToFocus(UUserWidget* Widget)
{
	//We want both the Game and UI to be focused because the player should still be able to move while the UI is open.
	//We also want cursor availability.
	FInputModeGameAndUI GameAndWidgetInputMode;
	
	DieRobotPlayerController->SetInputMode(GameAndWidgetInputMode);
	DieRobotPlayerController->EnableCursor();
	Widget->SetFocus();
}

void ADieRobotHUDBase::SetGameToFocus()
{
	//Setting the InputMode back to Game Only. Input mode is Changed in the Widget Blueprint in Event Preconstruct.
	FInputModeGameOnly GameOnlyInputMode;
	DieRobotPlayerController->SetInputMode(GameOnlyInputMode);
	DieRobotPlayerController->DisableCursor();
}

void ADieRobotHUDBase::HideAllChildWidgets(TArray<UUserWidget*> Widgets)
{
	if (Widgets.Num() > 0)
	{
		for (UUserWidget* Widget : Widgets)
		{
			HideWidget(Widget);
		}
	}
}

void ADieRobotHUDBase::ShowAllGameWidgets()
{
	ShowCrossHairWidget();
	ShowInventoryPanelWidget();
	ShowPlayerHealthWidget();
	ShowSeedaHealthWidget();
	ShowWaveDataWidget();
}

void ADieRobotHUDBase::OpenBuildPanelMenu()
{
	if (BuildMenuWidget)
	{
		BuildMenuWidget->SetVisibility(ESlateVisibility::Visible);
		
		FInputModeGameAndUI GameAndUIInputMode;
		//We may need to adjust this for focusing.
		DieRobotPlayerController->SetInputMode(GameAndUIInputMode);
		DieRobotPlayerController->SetFocusedUserWidget(BuildMenuWidget);
		DieRobotPlayerController->SetMouseLocation(GetCenterOfScreen().X, GetCenterOfScreen().Y);
		
		//Broadcasts to Player - Controlls Bool that starts Raycasting 
		bIsBuildMenuOpen.Broadcast(true);
		
	}
}

//Called from Building Component Icon in W_BuildingComponentIcon - Stays in Building State
void ADieRobotHUDBase::CloseBuildPanelMenu()
{
	//If the build menu is open, close it and disable the cursor (Build Mode, Raycasting)
	if (BuildMenuWidget && BuildMenuWidget->IsVisible())
	{
		//Broadcasts to Player - Starts Raycasting
		bIsBuildMenuOpen.Broadcast(false);
		//Hiding the Build Menu - NOT Destroying it.
		BuildMenuWidget->SetVisibility(ESlateVisibility::Hidden);
		
		//Setting the InputMode back to Game Only. Input mode is Changed in the Widget Blueprint in Event Preconstruct.
		FInputModeGameOnly GameOnlyInputMode;
		DieRobotPlayerController->SetInputMode(GameOnlyInputMode);
		DieRobotPlayerController->DisableCursor();
		
		HideDeleteBuildingComponentWidget();
	}
}

FVector2d ADieRobotHUDBase::GetCenterOfScreen()
{
	int32 ViewportSizeX, ViewportSizeY;
	GetOwningPlayerController()->GetViewportSize(ViewportSizeX, ViewportSizeY);

	return FVector2D(ViewportSizeX * 0.5f, ViewportSizeY * 0.5f);
}

FVector2d ADieRobotHUDBase::GetViewportSize()
{
	int32 ViewportSizeX, ViewportSizeY;
	GetOwningPlayerController()->GetViewportSize(ViewportSizeX, ViewportSizeY);

	return FVector2d(ViewportSizeX, ViewportSizeY);
}

void ADieRobotHUDBase::SwitchToDeathUI()
{
	/*
	 * Called from Player Controller.
	 * When Seeda Dies, it Broadcasts to the player to be Destroyed.
	 * When the player is Destroyed, it Broadcasts to the controller that the player is Destroyed.
	 */
	RootWidget->RemoveFromParent();
	
	if (DeathWidget)
	{
		DeathWidget->AddToViewport(1);
		DeathWidget->SetVisibility(ESlateVisibility::Visible);

		SetWidgetToFocus(DeathWidget);
	}
}

void ADieRobotHUDBase::SwitchToGameUI()
{
	if (DeathWidget)
	{
		DeathWidget->SetVisibility(ESlateVisibility::Hidden);
		//UE_LOG(LogTemp, Warning, TEXT("HUD - Hid the Death Widget"));	
	}
	if (RootWidget)
	{
		RootWidget->AddToViewport(1);
	}

	SetGameToFocus();
}

void ADieRobotHUDBase::ShowDeleteBuildingComponentWidget(float ViewportLocationX, float ViewportLocationY)
{
	//If widget is up, only update the position of the widget.
	//Parameters are the Screen location of the Impact Point

	//Shifting the Widget 100 Right and 100 Down from the Impact Point. On Screen.
	DeleteWidgetLocation.X = ViewportLocationX + DeleteBuildingComponentWidgetShiftX;
	DeleteWidgetLocation.Y = ViewportLocationY + DeleteBuildingComponentWidgetShiftY;
	
	if (DeleteBuildingComponentWidget)
	{
		
		DeleteBuildingComponentWidget->SetPositionInViewport(DeleteWidgetLocation);
		return;
	}

	//If the widget is not up, create it and add it to the viewport.
	if (DeleteBuildingComponentWidgetClass)
	{
		DeleteBuildingComponentWidget = CreateWidget<UUserWidget>(GetWorld(), DeleteBuildingComponentWidgetClass);
		if (DeleteBuildingComponentWidget)
		{
			DeleteBuildingComponentWidget->AddToViewport(1);

			//Constructing a 2D Vector to set the position of the Widget on the Viewport. For Some reason can not use the Delegate System with FVector2d Type.
			if (DeleteWidgetLocation != FVector2d(0, 0))
			{
				DeleteBuildingComponentWidget->SetPositionInViewport(DeleteWidgetLocation);
			}
		}
	}
}

void ADieRobotHUDBase::HideDeleteBuildingComponentWidget()
{
	//A Way to remove the DeleteBuildingComponentWidget from the Viewport.
	if (DeleteBuildingComponentWidget)
	{
		DeleteBuildingComponentWidget->RemoveFromParent();
		DeleteBuildingComponentWidget = nullptr;
	}
}

void ADieRobotHUDBase::HandleAmmoCounterVisibility(bool bShouldShowAmmoCounter)
{
	if (AmmoCounterWidget)
	{
		if (bShouldShowAmmoCounter)
		{
			AmmoCounterWidget->SetVisibility(ESlateVisibility::Visible);
		}
		else
		{
			AmmoCounterWidget->SetVisibility(ESlateVisibility::Hidden);
		}
	}
}

/*Tutorial*/

ETutorialState ADieRobotHUDBase::GetTutorialState()
{
	ADieRobotGameStateBase* DieRobotGameState = Cast<ADieRobotGameStateBase>(GetWorld()->GetGameState());
	if (DieRobotGameState)
	{
		HandleTutorialStateChanges(DieRobotGameState->TutorialState);
		return DieRobotGameState->TutorialState;
	}
	return ETutorialState::Default;
}

void ADieRobotHUDBase::HandleTutorialStateChanges(ETutorialState NewState)
{
	switch (NewState)
	{
	case ETutorialState::Wake1:
		GetRootWidgetChildrenWidgets();
		HideAllChildWidgets(RootWidgetChildrenWidgets);
		break;
	case ETutorialState::Wake2:
		ShowPlayerHealthWidget();
		ShowWidget(KBM_MovementControlsWidget);
		break;
	case ETutorialState::Wake3:
		ShowSeedaHealthWidget();
		HideWidget(KBM_MovementControlsWidget);
		break;
	case ETutorialState::Combat1:
		ShowWidget(KBM_CombatControlsWidget);
		ShowCrossHairWidget();
		break;
	case ETutorialState::Combat2:
		break;
	case ETutorialState::Parts1:
		UE_LOG(LogTemp, Warning, TEXT("Showing HUD."));
		HideWidget(KBM_CombatControlsWidget);
		ShowInventoryPanelWidget();
		break;
	case ETutorialState::Building1:
		ShowWidget(KBM_BuildControlsWidget);
		break;
	case ETutorialState::Building2:
		HideWidget(KBM_BuildControlsWidget);
		break;
	case ETutorialState::Building3:
		break;
	case ETutorialState::WaveStart:
		ShowWaveDataWidget();
		break;
	case ETutorialState::WaveComplete:
		break;
	case ETutorialState::TutorialComplete:
		ShowAllGameWidgets();
		break;
	case ETutorialState::Default:
		break;
	}
}

void ADieRobotHUDBase::GetRootWidgetChildrenWidgets()
{
	//UE_LOG(LogTemp, Warning, TEXT("Getting Root Widget Children"));
	if (RootWidget)
	{
		//UE_LOG(LogTemp, Warning, TEXT("HUD - Root Widget Exists"));
		if (Cast<UPanelWidget>(RootWidget->GetRootWidget()))
		{
			//Getting the Canvas Panel Widget that stores all the child widgets
			int32 NumOfChildWidgets = Cast<UPanelWidget>(RootWidget->GetRootWidget())->GetChildrenCount();
			for (int32 i = 0; i < NumOfChildWidgets; i++)
			{
				UUserWidget* ChildWidget = Cast<UUserWidget>(Cast<UPanelWidget>(RootWidget->GetRootWidget())->GetChildAt(i));
				if (ChildWidget)
				{
					RootWidgetChildrenWidgets.Add(ChildWidget);
					//UE_LOG(LogTemp, Warning, TEXT("Added Child Widget: %s"), *ChildWidget->GetName());
				}
			}
		}
	}
}

UUserWidget* ADieRobotHUDBase::GetWidgetByClassName(FString ClassName)
{
	if (RootWidgetChildrenWidgets.Num() > 0)
	{
		for (UUserWidget* Widget : RootWidgetChildrenWidgets)
		{
			//UE_LOG(LogTemp, Warning, TEXT("Widget Class Name: %s"), *Widget->GetClass()->GetName());
			if (Widget->GetClass()->GetName() == ClassName)
			{
				return Widget;
			}
		}
	}

	return nullptr;
}

UUserWidget* ADieRobotHUDBase::CreateVisibleWidget(const TSubclassOf<UUserWidget>& WidgetClass, int32 ZOrder)
{
	if (WidgetClass)
	{
		UUserWidget* VisibleWidget = CreateWidget<UUserWidget>(GetWorld(), WidgetClass);
		if (VisibleWidget)
		{
			VisibleWidget->AddToViewport(ZOrder);
			VisibleWidget->SetVisibility(ESlateVisibility::Visible);
			return VisibleWidget;
		}
	}
	//UE_LOG(LogTemp, Error, TEXT("Failed to create widget: %s"), *WidgetClass->GetName());
	return nullptr;
}

void ADieRobotHUDBase::ShowPlayerHealthWidget()
{
	//TODO::Chance this to the correct name of the Inventory Panel Widget
	FString PlayerHealthWidgetClassName = "W_HealthBar_C";
	//UE_LOG(LogTemp, Warning, TEXT("Player Health Widget Class Name: %s"), *PlayerHealthWidgetClassName);
	UUserWidget* Widget = GetWidgetByClassName(PlayerHealthWidgetClassName);
	ShowWidget(Widget);
	
}

void ADieRobotHUDBase::ShowSeedaHealthWidget()
{
	FString SeedaHealthWidgetClassName = "W_SeedaHealth_C";
	UUserWidget* Widget = GetWidgetByClassName(SeedaHealthWidgetClassName);
	ShowWidget(Widget);
	
}

void ADieRobotHUDBase::ShowWaveDataWidget()
{
	FString WaveDataWidgetClassName = "W_WaveInfo_C";
	UUserWidget* Widget = GetWidgetByClassName(WaveDataWidgetClassName);
	ShowWidget(Widget);
}

void ADieRobotHUDBase::HandleBossSpawned(AActor* BossActor)
{
	ABossBase* Boss = Cast<ABossBase>(BossActor);
	UBossHealthBar* BossHealthBar = Cast<UBossHealthBar>(BossHealthBarWidget);
	if (Boss && BossHealthBar)
	{
		//Setting Ref to Boss Actor Instance on Healthbar Widget
		BossHealthBar->BossActor = Boss;

		//Setting Boss Name on Health Bar Widget
		BossHealthBar->BossDisplayName = Boss->BossTechnicalName;

		ShowWidget(BossHealthBar);

		if (Boss->OnBossDeath.IsBound())
		{
			//Ensuring we dont have any bound functions to any bosses.
			Boss->OnBossDeath.RemoveDynamic(this, &ADieRobotHUDBase::HandleBossDeath);

			//Rebinding to the current boss.
			//When boss dies, we make the boss Health Bar Hidden.
			Boss->OnBossDeath.AddDynamic(this, &ADieRobotHUDBase::HandleBossDeath);
			
		}
	}
}

void ADieRobotHUDBase::ToggleBuildMenuStatusEffectDetails()
{
	if (BuildMenuWidget && BuildMenuWidget->IsVisible())
	{
		if (UBuildingComponentPanel* Panel = Cast<UBuildingComponentPanel>(BuildMenuWidget))
		{
			Panel->ToggleStatusEffectDetails();
		}
		else
		{
			//UE_LOG(LogTemp, Warning, TEXT("Could Not Cast to UBuildingComponentPanel from Build Menu Widget"));
		}
	}
	else
	{
		//UE_LOG(LogTemp, Warning, TEXT("Build Menu Widget is not visible, cannot toggle status effect details."));
	}
}

void ADieRobotHUDBase::ShowInventoryPanelWidget()
{
	FString InventoryWidgetClassName = "W_TopInventoryBar_C";
	//UE_LOG(LogTemp, Warning, TEXT("Player Inventory Widget Class Name: %s"), *InventoryWidgetClassName);
	UUserWidget* Widget = GetWidgetByClassName(InventoryWidgetClassName);
	if(Widget)
	{
		ShowWidget(Widget);
		//UE_LOG(LogTemp, Warning, TEXT("Player Inventory Widget Found"));
	}
	else
	{
		//UE_LOG(LogTemp, Warning, TEXT("Player Inventory Widget Not Found"));	
	}
}

void ADieRobotHUDBase::ShowCrossHairWidget()
{
	FString CrosshairWidgetClassName = "WBP_Crosshair_C";
	//UE_LOG(LogTemp, Warning, TEXT("Player Health Widget Class Name: %s"), *PlayerHealthWidgetClassName);
	UUserWidget* Widget = GetWidgetByClassName(CrosshairWidgetClassName);
	ShowWidget(Widget);
}
